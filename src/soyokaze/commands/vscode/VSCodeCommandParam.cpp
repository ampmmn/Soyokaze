#include "pch.h"
#include "VSCodeCommandParam.h"
#include "setting/Settings.h"
#include "utility/Path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace vscode {

CommandParam::CommandParam() : 
	mPrefix(_T("vscode"))
{
}


CommandParam::CommandParam(const CommandParam& rhs) : 
	mPrefix(rhs.mPrefix),
	mMinTriggerLength(rhs.mMinTriggerLength),
	mIsEnable(rhs.mIsEnable),
	mIsShowFullPath(rhs.mIsShowFullPath)
{
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (&rhs != this) {
		mPrefix = rhs.mPrefix;
		mMinTriggerLength = rhs.mMinTriggerLength;
		mIsEnable = rhs.mIsEnable;
		mIsShowFullPath = rhs.mIsShowFullPath;
	}
	return *this;
}

bool CommandParam::Save(Settings& settings) const
{
	settings.Set(_T("VSCodeSetting:IsEnable"), mIsEnable);
	settings.Set(_T("VSCodeSetting:Prefix"), mPrefix);
	settings.Set(_T("VSCodeSetting:MinTriggerLength"), mMinTriggerLength);
	settings.Set(_T("VSCodeSetting:IsShowFullPath"), mIsShowFullPath);
	return true;
}

bool CommandParam::Load(Settings& settings)
{
	mIsEnable = settings.Get(_T("VSCodeSetting:IsEnable"), true);
	mPrefix = settings.Get(_T("VSCodeSetting:Prefix"), _T("vscode"));
	mMinTriggerLength = settings.Get(_T("VSCodeSetting:MinTriggerLength"), 0);
	mIsShowFullPath = settings.Get(_T("VSCodeSetting:IsShowFullPath"), true);
	return true;
}

CString CommandParam::GetVSCodeExePath() const
{
	static Path path(Path::LOCALAPPDATA, _T("Programs\\Microsoft VS Code\\code.exe"));
	return (LPCTSTR)path;
}

bool CommandParam::HasPrefix() const
{
	return mPrefix.IsEmpty() == FALSE;
}

}}} // end of namespace launcherapp::commands::vscode

