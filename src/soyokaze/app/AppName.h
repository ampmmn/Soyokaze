#pragma once


#define APPNAME_ "Soyokaze"
#define APPNAME _T("Soyokaze")
#define APPNAME_LOWERCASE _T("soyokaze")
#define APP_EXENAME_ "soyokaze.exe"
#define APP_PROFILE_DIRNAME _T(".soyokaze")
#define APPLOGNAME APPNAME_LOWERCASE _T(".log")
#define COPYRIGHT_STR "ymgw.  All rights reserved."
#define LAUNCHER_APPID _T("{A04DA194-3C5E-454C-9C74-E9E740F85A4F}")

#include "app/override.h"

// {B8D984A3-84C3-43E7-92D6-B09298F88A42}
static constexpr GUID LAUNCHER_TOAST_CALLBACK_GUID = 
{
	0xb8d984a3, 0x84c3, 0x43e7, { 0x92, 0xd6, 0xb0, 0x92, 0x98, 0xf8, 0x8a, 0x42 }
};

