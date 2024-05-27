#include "pch.h"
#include "Calculator.h"
#include "utility/PythonDLL.h"
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

	PythonDLL mPython;
	tregex mRegSysFuncs;
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


// python.dllのパスを設定する
void Calculator::SetPythonDLLPath(const CString& dllPath)
{
	in->mPython.SetDLLPath(dllPath);
}

bool Calculator::Evaluate(const CString& src_, CString& result)
{
	// 実行を許可しない組み込み関数を含む場合は評価しない
	if (std::regex_search((LPCTSTR)src_, in->GetSysFuncRegex())) {
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

	return in->mPython.Evaluate(src, result);
}


}
}
}
