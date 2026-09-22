#include "pch.h"
#include "mainwindow/state/ShownState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/HiddenState.h"
#include "mainwindow/state/InputState.h"

using namespace launcherapp::mainwindow::state;

ShownState::ShownState(LauncherWindowStateContextIF* context) :
	LauncherWindowState(context)
{
}

void ShownState::OnActivate(bool isShowForce)
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

void ShownState::OnDeactivate()
{
	auto context = GetContext();
	context->ChangeState(std::make_unique<HiddenState>(context));
}

void ShownState::OnExecuteRequested()
{
	auto context = GetContext();
	context->ExecuteCurrentCommand();
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void ShownState::OnCancel()
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

void ShownState::OnTextChanged()
{
	auto context = GetContext();
	context->HandleTextChanged();
	if (context->HasKeyword()) {
		context->ChangeState(std::make_unique<InputState>(context));
	}
}

void ShownState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	auto context = GetContext();
	context->HandleQueryCompleted(result);
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
	else if (context->HasKeyword()) {
		context->ChangeState(std::make_unique<InputState>(context));
	}
}

bool ShownState::OnKeyInput(unsigned int keyCode)
{
	if (keyCode == VK_RETURN) {
		OnExecuteRequested();
		return true;
	}
	return false;
}
