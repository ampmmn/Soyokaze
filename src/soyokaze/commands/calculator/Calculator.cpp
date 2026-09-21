#include "pch.h"
#include "Calculator.h"
#include "python/PythonDLLLoader.h"
#include "setting/AppPreference.h"
#include "utility/Regex.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace calculator {

struct Calculator::PImpl
{
	PImpl()
	{
	}

	tregex& GetSysFuncRegex()
	{
		static bool isFirstCall = true;
		if (isFirstCall == false) {
			return mRegSysFuncs;
		}
		isFirstCall = false;

		// 実行を許可しない組み込み関数群
		std::vector<tstring> buildinFuncsions {
			_T("aiter"), _T("all"), _T("any"), _T("anext"),
				_T("breakpoint"), _T("bytearray"), _T("bytes"), _T("callable"),
				_T("classmethod"), _T("compile"), _T("complex"), _T("delattr"),
				_T("dict"), _T("dir"), _T("divmod"),
				_T("enumerate"), _T("eval"), _T("exec"),
				_T("filter"), _T("frozenset"),
				_T("getattr"), _T("globals"),
				_T("hasattr"), _T("hash"), _T("help"),
				_T("id"), _T("input"), _T("isinstance"), _T("issubclass"), _T("iter"),
				_T("list"), _T("locals"),
				_T("map"), _T("memoryview"),
				_T("next"),
				_T("object"), _T("open"),
				_T("print"), _T("property"),
				_T("range"), _T("repr"), _T("reversed"),
				_T("set"), _T("setattr"), _T("slice"), _T("sorted"), _T("staticmethod"), _T("str"), _T("sum"), _T("super"),
				_T("tuple"), _T("type"),
				_T("vars"),
				_T("zip"),
				_T("__import__"),
		};

		tstring pattern;
		for (auto& name : buildinFuncsions) {
			if(pattern.empty() == false) {
				pattern += _T("|");
			}
			pattern += name;
		}

		mRegSysFuncs = tregex(pattern);
		return mRegSysFuncs;
	}

	tregex mRegSysFuncs;

	/**
	  Pintを使わない従来の計算処理を実行する
	  @return true:成功 false:失敗
	  @param[in] src 評価する式
	  @param[out] result 評価結果
	*/
	bool EvaluateStandard(const CString& src_, CString& result)
	{
		// 実行を許可しない組み込み関数を含む場合は評価しない
		if (std::regex_search((LPCTSTR)src_, GetSysFuncRegex())) {
			return false;
		}

		CString src(src_);

		// 文字列を含むケースは対象外。ここでチェックしておく
		if (src.FindOneOf(_T("'\"")) != -1) {
			return false;
		}
		// 複数の文の実行は許可しない。
		int sep = src.Find(_T(';'));
		if (sep != -1) {
			src = src.Left(sep);
		}

		// インタープリタ側で拾ってしまうキーワードを無効化する(quit/exit/help)
		src.Replace(_T("quit"), _T(""));
		src.Replace(_T("exit"), _T(""));
		src.Replace(_T("copyright"), _T(""));
		src.Replace(_T("credits"), _T(""));
		src.Replace(_T("license"), _T(""));

		auto loader = PythonDLLLoader::Get();
		loader->Initialize();
		auto pythonLib = loader->GetLibrary();
		if (pythonLib == nullptr) {
			return false;
		}

		std::string tmpSrc;
		char* tmpResult = nullptr;
		bool isOK = pythonLib->EvalForCalculate(UTF2UTF(src, tmpSrc).c_str(), &tmpResult);

		if (tmpResult) {
			UTF2UTF(tmpResult, result);
			pythonLib->ReleaseBuffer(tmpResult);
		}
		else {
			result.Empty();
		}
		return isOK;
	}

	/**
	  Pintを使って単位付きの式を評価する
	  @return true:評価結果を採用できる false:従来処理へ移行
	  @param[in] src 評価する式
	  @param[out] result 評価結果
	*/
	bool EvaluatePint(const CString& src, CString& result)
	{
		if (AppPreference::Get()->GetSettings().Get(_T("Calculator:IsUsePint"), false) == false) {
			return false;
		}

		static const launcherapp::utility::Regex inputRegex(
			_T("^[0-9a-zA-Z_\\s.+\\-*/()]+$")
		);
		if (inputRegex.FullMatch(src) == false) {
			return false;
		}

		CString script;
		script.Format(
			_T("if \"__soyokaze_ureg\" not in globals():\n")
			_T("    from pint import UnitRegistry\n")
			_T("    __soyokaze_ureg = UnitRegistry()\n")
			_T("__soyokaze_result = __soyokaze_ureg.parse_expression(\"%s\").to_compact()\n")
			_T("__soyokaze_result = f\"{__soyokaze_result:~}\""),
			(LPCTSTR)src
		);

		auto loader = PythonDLLLoader::Get();
		loader->Initialize();
		auto pythonLib = loader->GetLibrary();
		if (pythonLib == nullptr) {
			return false;
		}

		std::string tmpSrc;
		char* tmpResult = nullptr;
		bool isOK = pythonLib->EvalScriptForCalculate(
			UTF2UTF(script, tmpSrc).c_str(),
			&tmpResult
		);
		if (isOK == false || tmpResult == nullptr) {
			if (tmpResult) {
				pythonLib->ReleaseBuffer(tmpResult);
			}
			return false;
		}

		CString pintResult;
		UTF2UTF(tmpResult, pintResult);
		pythonLib->ReleaseBuffer(tmpResult);

		// 単位を含まない結果は従来の電卓処理で扱う
		if (pintResult.Find(_T("dimensionless")) != -1) {
			return false;
		}

		pintResult.Replace(_T("'"), _T(""));
		result = pintResult;
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
}

bool Calculator::Evaluate(const CString& src_, CString& result)
{
	if (in->EvaluatePint(src_, result)) {
		return true;
	}
	return in->EvaluateStandard(src_, result);
}


}
}
}
