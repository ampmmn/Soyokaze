#include "pch.h"
#include "CharConverter.h"

#ifdef _MBCS
#error Unicode文字セットでビルドしてください
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace utility {

CharConverter::Exception::Exception() :
 	std::runtime_error("Character conversion error")
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CharConverter::CharConverter(int codePage) : mCodePage(codePage)
{
}

CharConverter::~CharConverter()
{
}

CString& CharConverter::Convert(const char* src, CString& dst, bool isFailIfInvalidChars)
{
	int cp = mCodePage;

	DWORD flags = isFailIfInvalidChars ? MB_ERR_INVALID_CHARS : 0;

	int requiredLen = MultiByteToWideChar(cp, flags, src, -1, NULL, 0);
	if (requiredLen == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		throw Exception();
	}

	MultiByteToWideChar(cp, flags, src, -1, dst.GetBuffer(requiredLen), requiredLen);
	dst.ReleaseBuffer();

	return dst;
}

CStringA& CharConverter::Convert(const CString& src, CStringA& dst)
{
	int cp = mCodePage;

	int requiredLen = WideCharToMultiByte(cp, 0, src, -1, NULL, 0, 0, 0);

	char* p = dst.GetBuffer(requiredLen);
	WideCharToMultiByte(cp, 0, src, -1, p, requiredLen, 0, 0);
	dst.ReleaseBuffer();

	return dst;
}

std::string& CharConverter::Convert(const CString& src, std::string& dst)
{
	int cp = mCodePage;

	int requiredLen = WideCharToMultiByte(cp, 0, src, -1, NULL, 0, 0, 0);

	dst.resize(requiredLen - 1);
	char* p = &dst.front();
	WideCharToMultiByte(cp, 0, src, -1, p, requiredLen, 0, 0);

	return dst;
}

// UTF-16 → UTF-8(wchar_t → char)
std::string CharConverter::UTF2UTF(const CStringW& src)
{
	std::string dst;
	return UTF2UTF(src, dst);
}

static std::string& utf2utf(const wchar_t* src, std::string& dst)
{
	int cp = CP_UTF8;

	int requiredLen = WideCharToMultiByte(cp, 0, src, -1, NULL, 0, 0, 0);

	dst.resize(requiredLen - 1);
	WideCharToMultiByte(cp, 0, src, -1, dst.data(), requiredLen, 0, 0);
	return dst;
}

std::string& CharConverter::UTF2UTF(const CStringW& src, std::string& dst)
{
	return utf2utf(src, dst);
}

std::string& CharConverter::UTF2UTF(const std::wstring& src, std::string& dst)
{
	return utf2utf(src.c_str(), dst);
}

// UTF-8 → UTF-16(char → wchar_t)
CStringW CharConverter::UTF2UTF(const std::string& src) 
{
	CStringW dst;
	return UTF2UTF(src, dst);
}

CStringW& CharConverter::UTF2UTF(const std::string& src, CStringW& dst)
{
	int cp = CP_UTF8;

	DWORD flags = MB_ERR_INVALID_CHARS;

	int requiredLen = MultiByteToWideChar(cp, flags, src.c_str(), -1, NULL, 0);
	if (requiredLen == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		dst.Empty();
		return dst;
	}

	MultiByteToWideChar(cp, flags, src.c_str(), -1, dst.GetBuffer(requiredLen), requiredLen);
	dst.ReleaseBuffer();

	return dst;
}

std::wstring& CharConverter::UTF2UTF(const std::string& src, std::wstring& dst)
{
	int cp = CP_UTF8;

	DWORD flags = MB_ERR_INVALID_CHARS;

	int requiredLen = MultiByteToWideChar(cp, flags, src.c_str(), -1, NULL, 0);
	if (requiredLen == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		dst.clear();
		return dst;
	}

	dst.resize(requiredLen-1);
	MultiByteToWideChar(cp, flags, src.c_str(), -1, const_cast<wchar_t*>(dst.data()), requiredLen);

	return dst;
}


int CharConverter::ScalarToUTF8(uint32_t scalar, char* dst)
{
	if (scalar <= 0x7F) {
		dst[0] = (char)(scalar & 0xFF);
		return 1;
	}
	if (scalar <= 0x7FF) {
		dst[0] = (char)(0xC0 | ((scalar >> 6) & 0x1F));
		dst[1] = (char)(0x80 | ((scalar     ) & 0x3F));
		return 2;
	}
	if (scalar <= 0xFFFF) {
		dst[0] = (char)(0xE0 | ((scalar >> 12) & 0x0F));
		dst[1] = (char)(0x80 | ((scalar >>  6) & 0x3F));
		dst[2] = (char)(0x80 | ((scalar      ) & 0x3F));
		return 3;
	}
	if (scalar <= 0x10FFFF) {
		dst[0] = (char)(0xF0 | ((scalar >> 18) & 0x07));
		dst[1] = (char)(0x80 | ((scalar >> 12) & 0x3F));
		dst[2] = (char)(0x80 | ((scalar >>  6) & 0x3F));
		dst[3] = (char)(0x80 | ((scalar      ) & 0x3F));
		return 4;
	}
	// UTF-8表現で5,6シーケンスになる文字は規格上ない
	return 0;
}


} // end of namespace utility
} // end of namespace launcherapp

