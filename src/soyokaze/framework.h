﻿#pragma once

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Windows ヘッダーから使用されていない部分を除外します。
#endif

#include "targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // 一部の CString コンストラクターは明示的です。

// 一般的で無視しても安全な MFC の警告メッセージの一部の非表示を解除します。
#define _AFX_ALL_WARNINGS

#include <afxwin.h>         // MFC のコアおよび標準コンポーネント
#include <afxext.h>         // MFC の拡張部分


#include <afxdisp.h>        // MFC オートメーション クラス



#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC の Internet Explorer 4 コモン コントロール サポート
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>             // MFC の Windows コモン コントロール サポート
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxcontrolbars.h>     // MFC におけるリボンとコントロール バーのサポート









#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif

#include "logger/Logger.h"
#include "utility/RefPtr.h"
#include "utility/CharConverter.h"

constexpr int MAX_PATH_NTFS = (32767+1);

#define NELEMENTS(ary)  (sizeof(ary) / sizeof(*(ary)))

#include <regex>
#include <string>
#include <fmt/core.h>

#ifdef _UNICODE
using tregex = std::wregex;
using tstring = std::wstring;
using tsmatch = std::wsmatch;
#else
using tregex = std::regex;
using tstring = std::string;
using tsmatch = std::smatch;
#endif

inline std::string& UTF2UTF(const CStringW& src, std::string& dst) {
	return launcherapp::utility::CharConverter::UTF2UTF(src, dst);
}
inline CStringW& UTF2UTF(const std::string& src, CStringW& dst) {
	return launcherapp::utility::CharConverter::UTF2UTF(src, dst);
}
inline std::wstring& UTF2UTF(const std::string& src, std::wstring& dst) {
	return launcherapp::utility::CharConverter::UTF2UTF(src, dst);
}
inline std::string& UTF2UTF(const std::wstring& src, std::string& dst) {
	return launcherapp::utility::CharConverter::UTF2UTF(src, dst);
}


// 構造体サイズ調査用
// https://oshiete.goo.ne.jp/qa/1823289.html
template <typename T, int> struct type_;
#define PRINT_SIZEOF(type) inline void size_of_(type_<type, sizeof(type)>) {}

// 以下のマクロをソースファイルに記述すると構造体のサイズが表示される(ビルド自体はエラーになる)
// PRINT_SIZEOF(BITMAPINFOHEADER)
