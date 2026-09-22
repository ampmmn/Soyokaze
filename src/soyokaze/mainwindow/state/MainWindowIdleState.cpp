#include "pch.h"
#include "mainwindow/state/MainWindowIdleState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/MainWindowSearchingState.h"

using namespace launcherapp::mainwindow::state;

IdleState::IdleState(LauncherWindowStateContextIF* context) :
	LauncherWindowState(context)
{
}

void IdleState::OnActivate(bool isShowForce)
{
	auto context = GetContext();
	if (isShowForce) {
		context->ShowWindowFromState();
	}
	else if (context->IsWindowActive()) {
		if (context->IsShowToggleEnabled()) {
			context->HideWindowFromState();
		}
	}
	else {
		context->ActivateVisibleWindow();
	}
}

void IdleState::OnDeactivate()
{
	auto context = GetContext();
	context->ChangeState(std::make_unique<HiddenState>(context));
}

void IdleState::OnExecuteRequested()
{
	auto context = GetContext();
	context->ExecuteCurrentCommand();
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void IdleState::OnCancel()
{
	auto context = GetContext();
	if (context->HasKeyword()) {
		context->ClearContent();
		context->SetFocusToEdit();
	}
	else {
		context->HideWindowFromState();
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void IdleState::OnTextChanged()
{
	auto context = GetContext();
	context->HandleTextChanged();
	if (context->HasKeyword()) {
		context->ChangeState(std::make_unique<SearchingState>(context));
	}
}

void IdleState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	auto context = GetContext();
	context->HandleQueryCompleted(result);
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
	else if (context->HasKeyword()) {
		context->ChangeState(std::make_unique<SearchingState>(context));
	}
}

bool IdleState::OnKeyInput(unsigned int keyCode)
{
	if (keyCode == VK_RETURN) {
		OnExecuteRequested();
		return true;
	}
	return false;
}
