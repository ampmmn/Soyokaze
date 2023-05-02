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
	return _T("yI—¹z");
}

BOOL ExitCommand::Execute()
{
	PostQuitMessage(0);
	return TRUE;
}

BOOL ExitCommand::Execute(const std::vector<CString>& args)
{
	// ˆø”w’è‚µ‚Ä‚à“®ì‚Í‚©‚í‚ç‚È‚¢
	return Execute();
}

CString ExitCommand::GetErrorString()
{
	return _T("");
}

