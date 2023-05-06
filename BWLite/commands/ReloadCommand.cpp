#include "pch.h"
#include "framework.h"
#include "ReloadCommand.h"
#include "CommandMap.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

ReloadCommand::ReloadCommand(CommandMap* pMap) :
	mCmdMapPtr(pMap)
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
	return mCmdMapPtr->Load();
}

BOOL ReloadCommand::Execute(const std::vector<CString>& args)
{
	// ˆø”Žw’è‚µ‚Ä‚à“®ì‚Í‚©‚í‚ç‚È‚¢
	return Execute();
}

CString ReloadCommand::GetErrorString()
{
	return _T("");
}

HICON ReloadCommand::GetIcon()
{
	return IconLoader::Get()->LoadReloadIcon();
}

BOOL ReloadCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

Command* ReloadCommand::Clone()
{
	return new ReloadCommand(mCmdMapPtr);
}

