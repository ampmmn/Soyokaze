#pragma once

#include <windows.h>
#include <string>

/**
 * @brief UTF-8 文字列を UTF-16 文字列に変換する
 * @param src UTF-8 文字列
 * @param dst UTF-16 文字列
 * @return 変換後の UTF-16 文字列
 */
inline std::wstring& utf2utf(const char* src, std::wstring& dst)
{
	int cp = CP_UTF8;
	DWORD flags = MB_ERR_INVALID_CHARS;

	// 必要なバッファサイズを計算
	int requiredLen = MultiByteToWideChar(cp, flags, src, -1, NULL, 0);
	if (requiredLen == 0 && GetLastError() == ERROR_NO_UNICODE_TRANSLATION) {
		dst.clear();
		return dst;
	}

	// バッファを確保して変換
	dst.resize(requiredLen - 1);
	MultiByteToWideChar(cp, flags, src, -1, const_cast<wchar_t*>(dst.data()), requiredLen);
	return dst;
}

inline std::wstring& utf2utf(const std::string& src, std::wstring& dst)
{
	return utf2utf(src.c_str(), dst);
}

inline std::string& utf2utf(const wchar_t* src, std::string& dst)
{
	int cp = CP_UTF8;

	int requiredLen = WideCharToMultiByte(cp, 0, src, -1, NULL, 0, 0, 0);

	dst.resize(requiredLen - 1);
	WideCharToMultiByte(cp, 0, src, -1, dst.data(), requiredLen, 0, 0);
	return dst;
}

inline std::string& utf2utf(const std::wstring& src, std::string& dst)
{
	return utf2utf(src.c_str(), dst);
}


