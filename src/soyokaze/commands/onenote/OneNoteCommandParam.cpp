#include "pch.h"
#include "OneNoteCommandParam.h"
#include "utility/RegistryKey.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace onenote {

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("OneNote:Prefix"), mPrefix);
	settings.Set(_T("OneNote:MinTriggerLength"), mMinTriggerLength);
	settings.Set(_T("OneNote:IsEnable"), mIsEnable);
	return true;
}

bool CommandParam::IsOneNoteSetupVerified()
{
	DWORD bootStatus;
	RegistryKey HKCU(HKEY_CURRENT_USER);
	if (HKCU.GetValue(_T("SOFTWARE\\Microsoft\\Office\\16.0\\OneNote"), _T("FirstBootStatus"), bootStatus) == false) {
		spdlog::info("[IsOneNoteSetupVerified] Failed to get FirstBootStatus key.");
		return false;
	}

	bool isVerified = bootStatus == 0x2000202;
	spdlog::info("[IsOneNoteSetupVerified] onenote setup : {}", isVerified);
	return isVerified;
}

bool CommandParam::Load(Settings& settings)
{
	mPrefix = settings.Get(_T("OneNote:Prefix"), _T(""));
	mMinTriggerLength = settings.Get(_T("OneNote:MinTriggerLength"), 5);
	mIsEnable = settings.Get(_T("OneNote:IsEnable"), false);

	// 初回起動時に一度だけ行う処理
	bool isVerified = settings.Get(_T("OneNote:IsOneNoteSetupVerified"), false);
	if (isVerified == false) {
		mIsEnable = IsOneNoteSetupVerified();
		settings.Set(_T("OneNote:IsOneNoteSetupVerified"), true);
	}

	return true;
}

}}} // end of namespace launcherapp::commands::onenote

