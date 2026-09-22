#include "pch.h"
#include "mainwindow/state/InputState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/HiddenState.h"
#include "mainwindow/state/ShownState.h"
#include "mainwindow/AppSound.h"

using namespace launcherapp::mainwindow::state;

InputState::InputState(LauncherWindowStateContextIF* context) :
	LauncherWindowState(context)
{
}

void InputState::OnActivate(bool isShowForce)
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

void InputState::OnDeactivate()
{
	auto context = GetContext();
	context->ChangeState(std::make_unique<HiddenState>(context));
}

void InputState::OnExecuteRequested()
{
	auto context = GetContext();
	context->ExecuteCurrentCommand();
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void InputState::OnCancel()
{
	auto context = GetContext();
	context->ClearContent();
	context->SetFocusToEdit();
	context->ChangeState(std::make_unique<ShownState>(context));
}

void InputState::OnContentCleared()
{
	auto context = GetContext();
	if (context->IsWindowVisibleFromState()) {
		context->ChangeState(std::make_unique<ShownState>(context));
	}
	else {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void InputState::OnTextChanged()
{
	auto context = GetContext();
	context->HandleTextChanged();
	if (context->HasKeyword() == false) {
		context->ChangeState(std::make_unique<ShownState>(context));
	}
}

void InputState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	auto context = GetContext();
	context->HandleQueryCompleted(result);
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
	else if (context->HasKeyword() == false) {
		context->ChangeState(std::make_unique<ShownState>(context));
	}
}

bool InputState::OnKeyInput(unsigned int keyCode)
{
	auto context = GetContext();
	if (keyCode == VK_UP || keyCode == VK_DOWN) {
		if (context->IsCandidateListEmpty()) {
			return false;
		}

		context->OffsetCandidateSelection(keyCode == VK_UP ? -1 : 1, true);
		AppSound::Get()->PlaySelectSound();
		context->UpdateCurrentCandidate();
		return true;
	}
	else if (keyCode == VK_TAB) {
		if (context->IsCandidateListEmpty()) {
			return true;
		}

		context->Complement();
		return true;
	}
	else if (keyCode == VK_RETURN) {
		if (context->IsCandidateListEmpty()) {
			return false;
		}
		OnExecuteRequested();
		return true;
	}
	else if (keyCode == VK_NEXT || keyCode == VK_PRIOR) {
		if (context->IsCandidateListEmpty()) {
			return false;
		}

		int offset = context->GetCandidateCountInPage();
		if (keyCode == VK_PRIOR) {
			offset = -offset;
		}
		context->OffsetCandidateSelection(offset, false);
		return true;
	}
	return false;
}

void InputState::OnCandidateSelectionChanged(int index)
{
	GetContext()->SelectCandidate(index);
}

void InputState::OnCandidateClicked()
{
	GetContext()->ReflectCurrentCandidate();
}

void InputState::OnCandidateDoubleClicked()
{
	OnExecuteRequested();
}
