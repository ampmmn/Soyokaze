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

String Arguments::Get(int index)
{
	return mArgV[index];
}

bool Arguments::Erase(int index)
{
	if (index < 0 || mArgV.size() <= (size_t)index) {
		return false;
	}

	mArgV.erase(mArgV.begin() + index);
	return true;
}

bool Arguments::Has(const char* optName)
{
	String optLower(optName);
	optLower.MakeLower();

	for (size_t i = 0; i < mArgV.size(); ++i) {
		auto arg = mArgV[i];
		arg.TrimLeft();
		arg.MakeLower();
		if (arg.Find(optLower) == 0) {
			return true;
		}
	}
	return false;
}

bool Arguments::GetValue(const char* optName, String& value)
{
	for (size_t i = 0; i < mArgV.size(); ++i) {
		if (mArgV[i].CompareNoCase(optName)== 0 && i+1 < mArgV.size()) {
			value = mArgV[i+1];
			return true;
		}
	}
	return false;
}

// bluewindと互換性があるオプション形式(/xxx=)で指定された値を取得
bool Arguments::GetBWOptValue(const char* optName, String& value)
{
	for (size_t i = 0; i < mArgV.size(); ++i) {

		String argPart = mArgV[i].Left((int)strlen(optName));
		if (argPart.CompareNoCase(optName) != 0) {
			continue;
		}

		auto arg = mArgV[i];
		int sepPos = arg.Find("=");
		if (sepPos == -1) {
			continue;
		}

		value = arg.Mid(sepPos+1);
		return true;
	}
	return false;
}
