#include "pch.h"
#include "MainWindowParamSearchingState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/MainWindowSearchingState.h"

using namespace launcherapp::mainwindow::state;

ParamSearchingState::ParamSearchingState(LauncherWindowStateContextIF* context) :
	LauncherWindowStateBase(context)
{
}

void ParamSearchingState::OnEnter()
{
	GetContext()->UpdateExtraCandidates();
}

void ParamSearchingState::OnExit()
{
	GetContext()->HideExtraCandidates();
}

void ParamSearchingState::OnDeactivate()
{
	GetContext()->HideExtraCandidates();
}

void ParamSearchingState::OnHideRequested()
{
	auto context = GetContext();
	context->ChangeState(std::make_unique<HiddenState>(context));
}

void ParamSearchingState::OnExecuteRequested()
{
	auto context = GetContext();
	context->ResolveExtraCandidate();
	context->ChangeState(std::make_unique<SearchingState>(context, false));
}

void ParamSearchingState::OnCancel()
{
	auto context = GetContext();
	context->ChangeState(std::make_unique<SearchingState>(context));
}

void ParamSearchingState::OnTextChanged()
{
	auto context = GetContext();
	context->UpdateInputState();
	if (context->HasKeyword() == false || context->CanStartParamSearching() == false) {
		context->ChangeState(std::make_unique<SearchingState>(context));
	}
	else {
		context->UpdateExtraCandidates();
	}
}

bool ParamSearchingState::OnKeyInput(unsigned int keyCode)
{
	auto context = GetContext();
	if (keyCode == VK_UP || keyCode == VK_DOWN) {
		if (context->IsExtraCandidateListEmpty()) {
			return true;
		}
		++mPendingSelectionChangeNotifications;
		context->OffsetExtraCandidateSelection(keyCode == VK_UP ? -1 : 1);
		return true;
	}
	if (keyCode == VK_TAB || keyCode == VK_RETURN) {
		if (context->IsExtraCandidateListEmpty()) {
			return true;
		}
		if (keyCode == VK_TAB) {
			context->ResolveExtraCandidate();
			// Tab確定後の検索結果でも追加候補Popupを表示できるようにする
			context->ChangeState(std::make_unique<SearchingState>(context, true));
		}
		else {
			// Enter確定後は追加候補Popupを再表示せず、通常の実行状態へ戻す
			OnExecuteRequested();
		}
		return true;
	}
	return false;
}

void ParamSearchingState::OnSelectionChanged()
{
	if (mPendingSelectionChangeNotifications > 0) {
		--mPendingSelectionChangeNotifications;
		return;
	}
	GetContext()->ChangeState(std::make_unique<SearchingState>(GetContext()));
}

void ParamSearchingState::OnWindowGeometryChanged()
{
	GetContext()->ChangeState(std::make_unique<SearchingState>(GetContext()));
}

void ParamSearchingState::OnExtraCandidateSelectionChanged()
{
}

void ParamSearchingState::OnExtraCandidateClicked()
{
	OnExecuteRequested();
}
