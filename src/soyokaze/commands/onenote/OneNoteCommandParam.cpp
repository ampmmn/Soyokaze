#include "pch.h"
#include "OneNoteCommandParam.h"

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

bool CommandParam::Load(Settings& settings)
{
	mPrefix = settings.Get(_T("OneNote:Prefix"), _T(""));
	mMinTriggerLength = settings.Get(_T("OneNote:MinTriggerLength"), 5);
	mIsEnable = settings.Get(_T("OneNote:IsEnable"), true);

	return true;
}

}}} // end of namespace launcherapp::commands::onenote

