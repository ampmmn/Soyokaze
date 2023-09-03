#pragma once

#include <string>

namespace soyokaze {
namespace utility {

class CharConverter
{
public:
	CharConverter(int codePage = CP_UTF8);
	~CharConverter();

	CString& Convert(const char* src, CString& dst);
	CStringA& Convert(const CString& src, CStringA& dst);
	std::string& Convert(const CString& src, std::string& dst);

	static int ScalarToUTF8(uint32_t scalar, char* dst);


protected:
	int mCodePage;
};


} // end of namespace utility
} // end of namespace soyokaze

