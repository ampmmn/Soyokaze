#include "pch.h"
#include "Calculator.h"
#include "python/PythonDLLLoader.h"
#include "utility/Path.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace calculator {

using LPMATHYPAD_EVALUATE = char* (*)(const char* expression);
using LPMATHYPAD_FREE_STRING = void (*)(char* result);

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
		if (std::regex_search((LPCTSTR)src_, GetSysFuncRegex())) {
			return false;
		}

		CString src(src_);
		if (src.FindOneOf(_T("'\"")) != -1) {
			return false;
		}
		int sep = src.Find(_T(';'));
		if (sep != -1) {
			src = src.Left(sep);
		}

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


}
}
}
