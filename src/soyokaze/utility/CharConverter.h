#pragma once

#include <string>
#include <stdexcept>

namespace launcherapp { namespace utility {

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
	
	static std::string UTF2UTF(const CStringW& src); 
	static std::string& UTF2UTF(const CStringW& src, std::string& dst); 
	static std::string& UTF2UTF(const std::wstring& src, std::string& dst); 
	
	static CStringW UTF2UTF(const std::string& src); 
	static CStringW& UTF2UTF(const std::string& src, CStringW& dst); 
	static std::wstring& UTF2UTF(const std::string& src, std::wstring& dst); 

	static int ScalarToUTF8(uint32_t scalar, char* dst);
	static int GetUTF8CharSize(const char* str);
	static int GetUTF8CharSize(char c);


protected:
	int mCodePage;
};


}} // end of namespace launcherapp::utility

