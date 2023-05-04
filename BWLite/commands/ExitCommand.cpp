#include "pch.h"
#include "framework.h"
#include "ExitCommand.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ExitCommand::ExitCommand()
{
}

ExitCommand::~ExitCommand()
{
}

CString ExitCommand::GetName()
{
	return _T("exit");
}

CString ExitCommand::GetDescription()
{
	return _T("ÅyèIóπÅz");
}

BOOL ExitCommand::Execute()
{
	PostQuitMessage(0);
	return TRUE;
}

BOOL ExitCommand::Execute(const std::vector<CString>& args)
{
	// à¯êîéwíËÇµÇƒÇ‡ìÆçÏÇÕÇ©ÇÌÇÁÇ»Ç¢
	return Execute();
}

CString ExitCommand::GetErrorString()
{
	return _T("");
}

HICON ExitCommand::GetIcon()
{
	return AfxGetApp()->LoadIcon(IDI_ICON2);
}

BOOL ExitCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

