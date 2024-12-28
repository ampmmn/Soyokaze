#pragma once

#include <vector>

class Arguments
{
public:
	Arguments(int argc, TCHAR* argv[]);
	~Arguments() = default;

	int GetCount();
	CString Get(int index);
	bool Erase(int index);


	bool Has(LPCTSTR optName);

	bool GetValue(LPCTSTR optName, CString& value);
	bool GetBWOptValue(LPCTSTR optName, CString& value);

protected:
	int mArgC;
	std::vector<CString> mArgV;

};


