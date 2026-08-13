#include "pch.h"
#include "CharConverter.h"
#include <cstring>
#include <cwchar>
#include <simdutf.h>

#ifdef _MBCS
#error Unicode文字セットでビルドしてください
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace utility {

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

static bool utf8ToUtf16(const char* src, size_t length, std::wstring& dst)
{
	// UTF-16の最大サイズは入力のバイト数で確保できる
	dst.resize(length);
	auto result = simdutf::convert_utf8_to_utf16_with_errors(
		src, length, reinterpret_cast<char16_t*>(dst.data()));
	if (result.error != simdutf::error_code::SUCCESS) {
		dst.clear();
		return false;
	}

	dst.resize(result.count);
	return true;
}

static void utf16ToUtf8(const wchar_t* src, size_t length, std::string& dst)
{
	auto requiredLength = simdutf::utf8_length_from_utf16_with_replacement(
		reinterpret_cast<const char16_t*>(src), length);
	dst.resize(requiredLength.count);
	if (dst.empty()) {
		return;
	}

	const size_t convertedLength = simdutf::convert_utf16_to_utf8_with_replacement(
		reinterpret_cast<const char16_t*>(src), length, dst.data());
	dst.resize(convertedLength);
}

CString& CharConverter::Convert(const char* src, CString& dst, bool isFailIfInvalidChars)
{
	int cp = mCodePage;
	if (cp == CP_UTF8) {
		std::wstring converted;
		if (utf8ToUtf16(src, strlen(src), converted)) {
			dst.SetString(converted.data(), (int)converted.size());
			return dst;
		}
		if (isFailIfInvalidChars) {
			throw Exception();
		}
	}

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
	if (cp == CP_UTF8) {
		std::string converted;
		utf16ToUtf8(src, wcslen(src), converted);
		dst.SetString(converted.data(), (int)converted.size());
		return dst;
	}

	int requiredLen = WideCharToMultiByte(cp, 0, src, -1, NULL, 0, 0, 0);

	char* p = dst.GetBuffer(requiredLen);
	WideCharToMultiByte(cp, 0, src, -1, p, requiredLen, 0, 0);
	dst.ReleaseBuffer();

	return dst;
}

std::string& CharConverter::Convert(const CString& src, std::string& dst)
{
	int cp = mCodePage;
	if (cp == CP_UTF8) {
		utf16ToUtf8(src, wcslen(src), dst);
		return dst;
	}

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
	utf16ToUtf8(src, wcslen(src), dst);
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

std::string& UTF2UTF(const wchar_t* src, std::string& dst)
{
	return utf2utf(src, dst);
}

// UTF-8 → UTF-16(char → wchar_t)
CStringW CharConverter::UTF2UTF(const std::string& src) 
{
	CStringW dst;
	return UTF2UTF(src, dst);
}

CStringW& CharConverter::UTF2UTF(const std::string& src, CStringW& dst)
{
	std::wstring converted;
	if (utf8ToUtf16(src.c_str(), strlen(src.c_str()), converted) == false) {
		dst.Empty();
		return dst;
	}

	dst.SetString(converted.data(), (int)converted.size());

	return dst;
}

std::wstring& CharConverter::UTF2UTF(const std::string& src, std::wstring& dst)
{
	if (utf8ToUtf16(src.c_str(), strlen(src.c_str()), dst) == false) {
		dst.clear();
		return dst;
	}

	return dst;
}


int CharConverter::ScalarToUTF8(uint32_t scalar, char* dst)
{
	if (scalar <= 0x7F) {
		if (dst) {
			dst[0] = (char)(scalar & 0xFF);
		}
		return 1;
	}
	if (scalar <= 0x7FF) {
		if (dst) {
			dst[0] = (char)(0xC0 | ((scalar >> 6) & 0x1F));
			dst[1] = (char)(0x80 | ((scalar     ) & 0x3F));
		}
		return 2;
	}
	if (scalar <= 0xFFFF) {
		if (dst) {
			dst[0] = (char)(0xE0 | ((scalar >> 12) & 0x0F));
			dst[1] = (char)(0x80 | ((scalar >>  6) & 0x3F));
			dst[2] = (char)(0x80 | ((scalar      ) & 0x3F));
		}
		return 3;
	}
	if (scalar <= 0x10FFFF) {
		if (dst) {
			dst[0] = (char)(0xF0 | ((scalar >> 18) & 0x07));
			dst[1] = (char)(0x80 | ((scalar >> 12) & 0x3F));
			dst[2] = (char)(0x80 | ((scalar >>  6) & 0x3F));
			dst[3] = (char)(0x80 | ((scalar      ) & 0x3F));
		}
		return 4;
	}
	// UTF-8表現で5,6シーケンスになる文字は規格上ない
	return 0;
}

// UTF-8文字のバイト数を判定する関数
int CharConverter::GetUTF8CharSize(const char* str)
{
	if (str == nullptr || *str == '\0') {
		return 0;
	}

	auto leadByte = static_cast<unsigned char>(str[0]);

	// UTF-8の先頭バイトの形式に基づいて判定
	if ((leadByte & 0x80) == 0x00) { // 1バイト文字 (ASCII)
		return 1;
	} else if ((leadByte & 0xE0) == 0xC0) { // 2バイト文字
		return 2;
	} else if ((leadByte & 0xF0) == 0xE0) { // 3バイト文字
		return 3;
	} else if ((leadByte & 0xF8) == 0xF0) { // 4バイト文字
		return 4;
	}
	return -1;
}

int CharConverter::GetUTF8CharSize(char c)
{
	return GetUTF8CharSize(&c);
}

}} // end of namespace launcherapp::utility

