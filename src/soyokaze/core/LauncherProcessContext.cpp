#include "pch.h"
#include "LauncherProcessContext.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

bool isPrimaryProcess = false;
bool isShutdownInProgress = false;

}


namespace launcherapp { namespace core {

LauncherProcessContext::LauncherProcessContext()
{
}

LauncherProcessContext::~LauncherProcessContext()
{
}

LauncherProcessContext* LauncherProcessContext::GetInstance()
{
	static LauncherProcessContext inst;
	return &inst;
}

// 先行プロセスである旨をセット
void LauncherProcessContext::MarkAsPrimaryProcess()
{
	isPrimaryProcess = true;
}

// シャットダウン中である旨をセット
void LauncherProcessContext::MarkShutdownInProgress()
{
	isShutdownInProgress = true;
}

// 先行プロセスか?
bool LauncherProcessContext::IsPrimaryProcess()
{
	return isPrimaryProcess;
}

// シャットダウン中か?
bool LauncherProcessContext::IsShutdownInProgress()
{
	return isShutdownInProgress;
}


}} // end of namespace launcherapp::core

