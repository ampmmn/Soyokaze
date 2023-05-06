#include "pch.h"
#include "framework.h"
#include "SettingCommand.h"
#include "SettingDialog.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SettingCommand::SettingCommand()
{
}

SettingCommand::~SettingCommand()
{
}

CString SettingCommand::GetName()
{
	return _T("setting");
}

CString SettingCommand::GetDescription()
{
	return _T("Åyê›íËÅz");
}

BOOL SettingCommand::Execute()
{
	SettingDialog dlg;
	if (dlg.DoModal() != IDOK) {
		return TRUE;
	}

	// ToDo: update setting

	return TRUE;
}

BOOL SettingCommand::Execute(const std::vector<CString>& args)
{
	// à¯êîéwíËÇµÇƒÇ‡ìÆçÏÇÕÇ©ÇÌÇÁÇ»Ç¢
	return Execute();
}

CString SettingCommand::GetErrorString()
{
	return _T("");
}

HICON SettingCommand::GetIcon()
{
	return IconLoader::Get()->LoadSettingIcon();

}


BOOL SettingCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

Command* SettingCommand::Clone()
{
	return new SettingCommand();
}

