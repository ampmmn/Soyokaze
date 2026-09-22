#include "pch.h"
#include "mainwindow/state/HiddenState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/InputState.h"
#include "mainwindow/state/ShownState.h"

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
			context->ChangeState(std::make_unique<InputState>(context));
		}
		else {
			context->ChangeState(std::make_unique<ShownState>(context));
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
