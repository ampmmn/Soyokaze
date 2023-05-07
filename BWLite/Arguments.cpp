#include "pch.h"
#include "framework.h"
#include "Arguments.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Arguments::Arguments(int argc, TCHAR* argv[]): mArgC(argc)
{
	mArgV.reserve(argc);
	for (int i = 0; i < argc; ++i) {
		mArgV.push_back(argv[i]);
	}
}


int Arguments::GetCount()
{
	return mArgC;
}

CString Arguments::Get(int index)
{
	return mArgV[index];
}

bool Arguments::GetValue(LPCTSTR optName, CString& value)
{
	for (size_t i = 0; i < mArgV.size(); ++i) {
		if (mArgV[i] == optName && i+1 < mArgV.size()) {
			value = mArgV[i+1];
			return true;
		}
	}
	return false;
}

// bluewindと互換性があるオプション形式(/xxx=)で指定された値を取得
bool Arguments::GetBWOptValue(LPCTSTR optName, CString& value)
{
	for (size_t i = 0; i < mArgV.size(); ++i) {

		auto arg = mArgV[i];
		if (_tcsncmp(optName, arg, _tcslen(optName)) != 0) {
			continue;
		}

		int sepPos = arg.Find(_T("="));
		if (sepPos == -1) {
			continue;
		}

		value = arg.Mid(sepPos+1);
		AfxMessageBox(value);
		return true;
	}
	return false;
}
