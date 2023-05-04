#pragma once

#include <vector>

class Arguments
{
public:
	Arguments(int argc, TCHAR* argv[]);
	~Arguments() = default;

	bool GetValue(LPCTSTR optName, CString& value);

protected:
	int mArgC;
	std::vector<CString> mArgV;

};


