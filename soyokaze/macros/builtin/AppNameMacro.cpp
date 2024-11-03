#include "pch.h"
#include "AppNameMacro.h"
#include "app/AppName.h"
#include "utility/VersionInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace macros {
namespace builtin {

REGISTER_LAUNCHERMACRO(AppNameMacro)

AppNameMacro::AppNameMacro()
{
	mName = _T("app");
}

AppNameMacro::~AppNameMacro()
{
}

bool AppNameMacro::Evaluate(const std::vector<CString>& args, CString& result)
{
	if (args.size() == 0) {
		result = APPNAME;
		return true;
	}

	CString param = args[0];
	if (param == _T("name")) {
		result = APPNAME;
	}
	else if (param == _T("version")) {
		// バージョン情報を取得
		VersionInfo::GetVersionInfo(result);
	}
	else if (param == _T("builddate")) {
		// ビルド日時
		CTime tmBuildDate;
		VersionInfo::GetBuildDateTime(tmBuildDate);
		result = tmBuildDate.Format(_T("%F"));
	}
	else if (param == _T("buildtime")) {
		// ビルド日時
		CTime tmBuildDate;
		VersionInfo::GetBuildDateTime(tmBuildDate);
		result = tmBuildDate.Format(_T("%T"));
	}


	return true;
}


}
}
}
