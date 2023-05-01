#include "pch.h"
#include "framework.h"
#include "ReloadCommand.h"
#include "CommandMap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ReloadCommand::ReloadCommand(CommandMap* pMap) :
	m_pCommandMap(pMap)
{
}

ReloadCommand::~ReloadCommand()
{
}

CString ReloadCommand::GetName()
{
	return _T("reload");
}

CString ReloadCommand::GetDescription()
{
	return _T("Ý’è‚ÌƒŠƒ[ƒh");
}

BOOL ReloadCommand::Execute()
{
	return m_pCommandMap->Load();
}

CString ReloadCommand::GetErrorString()
{
	return _T("");
}

