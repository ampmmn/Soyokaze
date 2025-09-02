#pragma once

#include <vector>

class Arguments
{
public:
	Arguments(int argc, TCHAR* argv[]);
	~Arguments() = default;

	int GetCount();
	String Get(int index);
	bool Erase(int index);


	bool Has(const char* optName);

	bool GetValue(const char* optName, String& value);
	bool GetBWOptValue(const char* optName, String& value);

protected:
	int mArgC;
	std::vector<String> mArgV;

};


