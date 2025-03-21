#pragma once

#include <string>
#include <stdexcept>

namespace launcherapp {
namespace utility {

class CharConverter
{
public:
	class Exception : public std::runtime_error {
	public:
			Exception();
	};

public:
	CharConverter(int codePage = CP_UTF8);
	~CharConverter();

	CString& Convert(const char* src, CString& dst, bool isFailIfInvalidChars = false);
	CStringA& Convert(const CString& src, CStringA& dst);
	std::string& Convert(const CString& src, std::string& dst);

	static int ScalarToUTF8(uint32_t scalar, char* dst);


protected:
	int mCodePage;
};


} // end of namespace utility
} // end of namespace launcherapp

