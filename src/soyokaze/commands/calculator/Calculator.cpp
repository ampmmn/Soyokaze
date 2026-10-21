#include "pch.h"
#include "Calculator.h"
#include "python/PythonDLLLoader.h"
#include "utility/Path.h"
#include <algorithm>
#include <cwctype>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace calculator {

using LPMATHYPAD_EVALUATE = char* (*)(const char* expression);
using LPMATHYPAD_FREE_STRING = void (*)(char* result);

namespace {

// 電卓機能として評価する式の最大文字数
constexpr int MAX_EXPRESSION_LENGTH = 256;

// 数値リテラルに使用できる最大桁数
constexpr int MAX_LITERAL_DIGITS = 30;

// 数値リテラル(指数部)に使用できる最大桁数
constexpr int MAX_LITERAL_EXPONENT_DIGITS = 3;

// 数値リテラル(指数部)に使用できる最大値
constexpr int MAX_LITERAL_EXPONENT = 100;

// べき乗の指数に使用できる最大値
// (これを超えると評価結果が指数的に大きくなり、リソースを過剰に消費するため)
constexpr int MAX_POWER_EXPONENT = 100;

/**
  電卓機能で使用を許可する識別子の一覧を取得する
  関数-local staticとして保持するため初期化はスレッドセーフ
  @return 許可する識別子の一覧
*/
const std::vector<tstring>& GetAllowedIdentifiers()
{
	static const std::vector<tstring> identifiers = {
		// mathモジュールから提供される関数
		_T("acos"), _T("acosh"), _T("asin"), _T("asinh"),
		_T("atan"), _T("atan2"), _T("atanh"),
		_T("cbrt"), _T("ceil"), _T("copysign"),
		_T("cos"), _T("cosh"), _T("degrees"), _T("dist"),
		_T("erf"), _T("erfc"), _T("exp"), _T("exp2"), _T("expm1"),
		_T("fabs"), _T("floor"), _T("fmod"), _T("frexp"),
		_T("fsum"), _T("gamma"), _T("gcd"), _T("hypot"),
		_T("isclose"), _T("isfinite"), _T("isinf"), _T("isnan"), _T("isqrt"),
		_T("lcm"), _T("ldexp"), _T("lgamma"), _T("log"), _T("log10"),
		_T("log1p"), _T("log2"), _T("modf"), _T("nextafter"),
		_T("pow"), _T("prod"), _T("radians"), _T("remainder"),
		_T("sin"), _T("sinh"), _T("sqrt"), _T("sumprod"), _T("tan"),
		_T("tanh"), _T("trunc"), _T("ulp"),
		// 定数
		_T("pi"), _T("e"), _T("tau"), _T("inf"), _T("nan"),
		// 電卓機能のために許可する組み込み関数
		_T("abs"), _T("bin"), _T("float"), _T("hex"), _T("int"),
		_T("max"), _T("min"), _T("oct"), _T("round"),
		// mathモジュールを参照するための名前
		_T("math"),
	};
	return identifiers;
}

/**
  識別子が電卓機能で使用を許可されているかどうかを判定する
  @return true:許可されている false:許可されていない
  @param[in] name 判定する識別子
*/
bool IsAllowedIdentifier(const tstring& name)
{
	const auto& identifiers = GetAllowedIdentifiers();
	return std::find(identifiers.begin(), identifiers.end(), name) != identifiers.end();
}

} // namespace

struct Calculator::PImpl
{
	PImpl()
	{
	}

	/**
	  式を電卓機能として評価してよいかどうかを検証する
	  四則演算とmathモジュールの関数・定数のみを許可し、
	  任意のコードを実行できる式を拒否する
	  @return true:評価してよい false:評価しない
	  @param[in] src 検証する式
	*/
	static bool ValidateExpression(const CString& src)
	{
		int len = src.GetLength();
		if (len == 0 || len > MAX_EXPRESSION_LENGTH) {
			return false;
		}

		int pos = 0;
		// 直前のトークンが値(数値/識別子)かどうか
		bool prevIsValue = false;
		// 直前のトークンが属性アクセス('.')かどうか
		bool prevIsDot = false;

		while (pos < len) {
			TCHAR c = src[pos];

			// 空白は読み飛ばす
			if (std::iswspace(c)) {
				pos++;
				continue;
			}

			// 小数点/属性アクセス
			if (c == _T('.')) {
				if (prevIsValue) {
					// 属性アクセス
					pos++;
					prevIsValue = false;
					prevIsDot = true;
					continue;
				}
				// 数値(小数)
				pos = ScanNumber(src, pos);
				if (pos < 0) {
					return false;
				}
				prevIsValue = true;
				prevIsDot = false;
				continue;
			}

			// 数値リテラル
			if (std::iswdigit(c)) {
				if (prevIsValue) {
					// 値が連続している
					return false;
				}
				pos = ScanNumber(src, pos);
				if (pos < 0) {
					return false;
				}
				prevIsValue = true;
				prevIsDot = false;
				continue;
			}

			// 識別子(関数名・定数名・属性名)
			// アンダースコアで始まる識別子(dunder)は許可しない
			if (std::iswalpha(c)) {
				if (prevIsValue && prevIsDot == false) {
					// 値が連続している
					return false;
				}
				pos = ScanIdentifier(src, pos);
				if (pos < 0) {
					return false;
				}
				prevIsValue = true;
				prevIsDot = false;
				continue;
			}

			// べき乗
			if (c == _T('*') && pos + 1 < len && src[pos + 1] == _T('*')) {
				pos = ScanPower(src, pos + 2);
				if (pos < 0) {
					return false;
				}
				prevIsValue = true;
				prevIsDot = false;
				continue;
			}

			// 四則演算・剰余・括弧・引数区切り
			if (c == _T('+') || c == _T('-') || c == _T('*') || c == _T('/') ||
				c == _T('%') || c == _T('(') || c == _T(')') || c == _T(',')) {
				pos++;
				prevIsValue = false;
				prevIsDot = false;
				continue;
			}

			// 上記以外の文字(引用符・セミコロン・アンダースコア等)は許可しない
			return false;
		}
		return true;
	}

	/**
	  数値リテラルを読み取る
	  @return 読み取り後の位置 失敗時は-1
	  @param[in] src 読み取る対象の文字列
	  @param[in] pos 読み取り開始位置
	  @param[out] value 読み取った数値( nullptrの場合は読み取らない)
	*/
	static int ScanNumber(const CString& src, int pos, double* value = nullptr)
	{
		int len = src.GetLength();
		double result = 0;

		// 基数接頭辞(0x/0o/0b)に対応する
		if (pos + 1 < len && src[pos] == _T('0')) {
			TCHAR base = src[pos + 1];
			if (base == _T('x') || base == _T('X') || base == _T('o') ||
				base == _T('O') || base == _T('b') || base == _T('B')) {
				int radix = 16;
				if (base == _T('o') || base == _T('O')) {
					radix = 8;
				}
				else if (base == _T('b') || base == _T('B')) {
					radix = 2;
				}

				int digitPos = pos + 2;
				int digits = 0;
				while (digitPos < len && IsBaseDigit(src[digitPos], base)) {
					result = result * radix + ToDigit(src[digitPos]);
					digits++;
					digitPos++;
				}
				if (digits == 0 || digits > MAX_LITERAL_DIGITS) {
					return -1;
				}
				if (value != nullptr) {
					*value = result;
				}
				return digitPos;
			}
		}

		int intDigits = 0;
		while (pos < len && std::iswdigit(src[pos])) {
			result = result * 10 + ToDigit(src[pos]);
			intDigits++;
			pos++;
		}

		int fracDigits = 0;
		if (pos < len && src[pos] == _T('.')) {
			pos++;
			double scale = 0.1;
			while (pos < len && std::iswdigit(src[pos])) {
				result += ToDigit(src[pos]) * scale;
				scale *= 0.1;
				fracDigits++;
				pos++;
			}
		}

		if (intDigits == 0 && fracDigits == 0) {
			// 数値が存在しない
			return -1;
		}
		if (intDigits > MAX_LITERAL_DIGITS || fracDigits > MAX_LITERAL_DIGITS) {
			return -1;
		}

		// 指数部
		if (pos < len && (src[pos] == _T('e') || src[pos] == _T('E'))) {
			int exponentPos = pos + 1;
			bool negative = false;
			if (exponentPos < len && (src[exponentPos] == _T('+') || src[exponentPos] == _T('-'))) {
				negative = (src[exponentPos] == _T('-'));
				exponentPos++;
			}
			int exponentDigits = 0;
			int exponent = 0;
			while (exponentPos < len && std::iswdigit(src[exponentPos])) {
				exponent = exponent * 10 + ToDigit(src[exponentPos]);
				exponentDigits++;
				exponentPos++;
			}
			if (exponentDigits == 0) {
				// 指数部の数値がない場合は、'e'を次の識別子として扱う
				// (例: "2e" はPython側で構文エラーとなる)
				if (value != nullptr) {
					*value = result;
				}
				return pos;
			}
			if (exponentDigits > MAX_LITERAL_EXPONENT_DIGITS) {
				return -1;
			}
			if (exponent > MAX_LITERAL_EXPONENT) {
				return -1;
			}
			result *= std::pow(10.0, negative ? -exponent : exponent);
			pos = exponentPos;
		}

		if (value != nullptr) {
			*value = result;
		}
		return pos;
	}

	/**
	  基数接頭辞で指定された基数で解釈できる数字かどうかを判定する
	  @return true:解釈できる false:解釈できない
	  @param[in] c 判定する文字
	  @param[in] base 基数の指定(0x/0o/0b)
	*/
	static bool IsBaseDigit(TCHAR c, TCHAR base)
	{
		if (c >= _T('0') && c <= _T('9')) {
			if (base == _T('b') || base == _T('B')) {
				return c <= _T('1');
			}
			if (base == _T('o') || base == _T('O')) {
				return c <= _T('7');
			}
			return true;
		}
		if (base == _T('x') || base == _T('X')) {
			if (c >= _T('a') && c <= _T('f')) {
				return true;
			}
			if (c >= _T('A') && c <= _T('F')) {
				return true;
			}
		}
		return false;
	}

	/**
	  数字文字列を数値に変換する
	  @return 変換後の数値
	  @param[in] c 変換する文字
	*/
	static int ToDigit(TCHAR c)
	{
		return c - _T('0');
	}

	/**
	  識別子(関数名・定数名・属性名)を読み取る
	  @return 読み取り後の位置 失敗時は-1
	  @param[in] src 読み取る対象の文字列
	  @param[in] pos 読み取り開始位置
	*/
	static int ScanIdentifier(const CString& src, int pos)
	{
		int len = src.GetLength();
		int start = pos;
		while (pos < len && std::iswalnum(src[pos])) {
			pos++;
		}
		if (pos == start) {
			return -1;
		}
		if (IsAllowedIdentifier(tstring(src.Mid(start, pos - start))) == false) {
			return -1;
		}
		return pos;
	}

	/**
	  べき乗演算(**)の指数部分を読み取る
	  指数は数値リテラル(括弧で囲んだものを含む)のみを許可する
	  指数の直後が再度べき乗になっている場合は、
	  評価結果が指数的に大きくなるため拒否する
	  @return 読み取り後の位置 失敗時は-1
	  @param[in] src 読み取る対象の文字列
	  @param[in] pos 読み取り開始位置
	*/
	static int ScanPower(const CString& src, int pos)
	{
		int len = src.GetLength();

		// 空白を読み飛ばす
		while (pos < len && std::iswspace(src[pos])) {
			pos++;
		}

		// 括弧で囲まれている場合がある
		bool parenthesized = false;
		if (pos < len && src[pos] == _T('(')) {
			parenthesized = true;
			pos++;
			while (pos < len && std::iswspace(src[pos])) {
				pos++;
			}
		}

		// 符号
		bool negative = false;
		if (pos < len && (src[pos] == _T('+') || src[pos] == _T('-'))) {
			negative = (src[pos] == _T('-'));
			pos++;
			while (pos < len && std::iswspace(src[pos])) {
				pos++;
			}
		}

		// 指数の値は数値リテラルでなければならない
		double value = 0;
		pos = ScanNumber(src, pos, &value);
		if (pos < 0) {
			return -1;
		}
		if (negative) {
			value = -value;
		}
		if (value > MAX_POWER_EXPONENT || value < -MAX_POWER_EXPONENT) {
			return -1;
		}

		if (parenthesized) {
			while (pos < len && std::iswspace(src[pos])) {
				pos++;
			}
			if (pos >= len || src[pos] != _T(')')) {
				return -1;
			}
			pos++;
		}

		// 指数がさらにべき乗を含む場合は評価結果が巨大になるため許可しない
		if (pos + 1 < len && src[pos] == _T('*') && src[pos + 1] == _T('*')) {
			return -1;
		}
		return pos;
	}

	bool mIsUseStandardEvaluate{false};
	bool mIsUseMathypadEvaluate{false};
	bool mIsMathypadInitialized{false};
	HMODULE mMathypadDll{nullptr};
	LPMATHYPAD_EVALUATE mMathypadEvaluate{nullptr};
	LPMATHYPAD_FREE_STRING mMathypadFreeString{nullptr};

	/**
	  MathyPad DLLを初期化する
	  @return true:利用可能 false:利用不可
	*/
	bool InitializeMathypad()
	{
		if (mIsMathypadInitialized) {
			return mMathypadDll != nullptr;
		}
		mIsMathypadInitialized = true;

		Path dllPath(Path::MODULEFILEDIR);
		dllPath.Append(_T("mathypad.dll"));
		mMathypadDll = LoadLibrary(dllPath);
		if (mMathypadDll == nullptr) {
			return false;
		}

		mMathypadEvaluate = reinterpret_cast<LPMATHYPAD_EVALUATE>(
			GetProcAddress(mMathypadDll, "mathypad_evaluate"));
		mMathypadFreeString = reinterpret_cast<LPMATHYPAD_FREE_STRING>(
			GetProcAddress(mMathypadDll, "mathypad_free_string"));
		if (mMathypadEvaluate == nullptr || mMathypadFreeString == nullptr) {
			FreeLibrary(mMathypadDll);
			mMathypadDll = nullptr;
			mMathypadEvaluate = nullptr;
			mMathypadFreeString = nullptr;
			return false;
		}

		return true;
	}

	/**
	  MathyPad DLLを解放する
	*/
	void FinalizeMathypad()
	{
		mMathypadEvaluate = nullptr;
		mMathypadFreeString = nullptr;
		if (mMathypadDll != nullptr) {
			FreeLibrary(mMathypadDll);
			mMathypadDll = nullptr;
		}
	}

	/**
	  標準電卓で式を評価する
	  @return true:成功 false:失敗
	  @param[in] src_ 評価する式
	  @param[out] result 評価結果
	*/
	bool EvaluateStandard(const CString& src_, CString& result)
	{
		// 電卓機能として評価してよい式かどうかを検証する
		// (検証を通過した式には任意のコードを実行する手段が含まれない)
		if (ValidateExpression(src_) == false) {
			return false;
		}

		auto loader = PythonDLLLoader::Get();
		loader->Initialize();
		auto pythonLib = loader->GetLibrary();
		if (pythonLib == nullptr) {
			return false;
		}

		std::string tmpSrc;
		char* tmpResult = nullptr;
		bool isOK = pythonLib->EvalForCalculate(UTF2UTF(src_, tmpSrc).c_str(), &tmpResult);
		if (tmpResult != nullptr) {
			UTF2UTF(tmpResult, result);
			pythonLib->ReleaseBuffer(tmpResult);
		}
		else {
			result.Empty();
		}
		return isOK;
	}

	/**
	  MathyPadで単位付きの式を評価する
	  @return true:成功 false:失敗
	  @param[in] src 評価する式
	  @param[out] result 評価結果
	*/
	bool EvaluateMathypad(const CString& src, CString& result)
	{
		if (mIsUseMathypadEvaluate == false || InitializeMathypad() == false) {
			return false;
		}

		// "N days later" "N days ago" 形式は日時候補の機能で扱うため、電卓の結果としては除外する
		CString lower(src);
		lower.MakeLower();
		if (lower.Find(_T("later")) >= 0 || lower.Find(_T("ago")) >= 0) {
			return false;
		}

		std::string expression;
		UTF2UTF(std::wstring(src), expression);
		char* mathypadResult = mMathypadEvaluate(expression.c_str());
		if (mathypadResult == nullptr) {
			return false;
		}

		std::string resultUtf8(mathypadResult);
		mMathypadFreeString(mathypadResult);
		UTF2UTF(resultUtf8, result);
		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



Calculator::Calculator() : in(std::make_unique<PImpl>())
{
}

Calculator::~Calculator()
{
	in->FinalizeMathypad();
}

void Calculator::UseStandardEvaluate(bool use)
{
	in->mIsUseStandardEvaluate = use;
}

bool Calculator::IsUseStandardEvaluate()
{
	return in->mIsUseStandardEvaluate;
}

void Calculator::UseMathypadEvaluate(bool use)
{
	in->mIsUseMathypadEvaluate = use;
}


bool Calculator::Evaluate(const CString& src, CString& result) {
	if (in->mIsUseStandardEvaluate && in->EvaluateStandard(src, result)) {
		return true;
	}
	return in->EvaluateMathypad(src, result);
}

bool Calculator::IsValidExpression(const CString& src)
{
	return PImpl::ValidateExpression(src);
}


}
}
}
