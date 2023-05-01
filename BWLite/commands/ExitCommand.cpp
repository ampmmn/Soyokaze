#include "pch.h"
#include "framework.h"
#include "ExitCommand.h"

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

CString ExitCommand::GetErrorString()
{
	return _T("");
}

