#include "pch.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowSearchingState.h"
#include "mainwindow/state/MainWindowIdleState.h"

using namespace launcherapp::mainwindow::state;

HiddenState::HiddenState(LauncherWindowStateContextIF* context) :
	LauncherWindowState(context)
{
}

void HiddenState::OnEnter()
{
	auto context = GetContext();
	if (context->IsWindowVisibleFromState()) {
		context->HideWindowFromState();
	}
}

void HiddenState::OnActivate(bool isShowForce)
{
	UNREFERENCED_PARAMETER(isShowForce);
	auto context = GetContext();
	context->ShowWindowFromState();
	if (context->IsWindowVisibleFromState()) {
		if (context->HasKeyword()) {
			context->ChangeState(std::make_unique<SearchingState>(context));
		}
		else {
			context->ChangeState(std::make_unique<IdleState>(context));
		}
	}
}

void HiddenState::OnExecuteRequested()
{
	GetContext()->ExecuteCurrentCommand();
}

void HiddenState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	GetContext()->HandleQueryCompleted(result);
}
