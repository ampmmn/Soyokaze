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


