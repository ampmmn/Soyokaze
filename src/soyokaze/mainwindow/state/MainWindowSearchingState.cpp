#include "pch.h"
#include "mainwindow/state/MainWindowSearchingState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/MainWindowIdleState.h"
#include "mainwindow/AppSound.h"

using namespace launcherapp::mainwindow::state;

SearchingState::SearchingState(LauncherWindowStateContextIF* context) :
	LauncherWindowState(context)
{
}

void SearchingState::OnActivate(bool isShowForce)
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

void SearchingState::OnDeactivate()
{
	auto context = GetContext();
	context->ChangeState(std::make_unique<HiddenState>(context));
}

void SearchingState::OnExecuteRequested()
{
	auto context = GetContext();
	context->ExecuteCurrentCommand();
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void SearchingState::OnCancel()
{
	auto context = GetContext();
	context->ClearContent();
	context->SetFocusToEdit();
	context->ChangeState(std::make_unique<IdleState>(context));
}

void SearchingState::OnContentCleared()
{
	auto context = GetContext();
	if (context->IsWindowVisibleFromState()) {
		context->ChangeState(std::make_unique<IdleState>(context));
	}
	else {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void SearchingState::OnTextChanged()
{
	auto context = GetContext();
	context->HandleTextChanged();
	if (context->HasKeyword() == false) {
		context->ChangeState(std::make_unique<IdleState>(context));
	}
}

void SearchingState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	auto context = GetContext();
	context->HandleQueryCompleted(result);
	if (context->IsWindowVisibleFromState() == false) {
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
	else if (context->HasKeyword() == false) {
		context->ChangeState(std::make_unique<IdleState>(context));
	}
}

bool SearchingState::OnKeyInput(unsigned int keyCode)
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

void SearchingState::OnCandidateSelectionChanged(int index)
{
	GetContext()->SelectCandidate(index);
}

void SearchingState::OnCandidateClicked()
{
	GetContext()->ReflectCurrentCandidate();
}

void SearchingState::OnCandidateDoubleClicked()
{
	OnExecuteRequested();
}
