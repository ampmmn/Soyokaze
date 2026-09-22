#include "pch.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowSearchingState.h"
#include "mainwindow/state/MainWindowIdleState.h"

using namespace launcherapp::mainwindow::state;

HiddenState::HiddenState(LauncherWindowStateContextIF* context) :
	LauncherWindowStateBase(context)
{
}

void HiddenState::OnEnter()
{
	auto context = GetContext();
	// State遷移元の処理によって表示されたままの場合も、非表示状態を保証する
	if (context->IsWindowVisibleFromState()) {
		context->HideWindowFromState();
	}
}

void HiddenState::OnActivate(bool isShowForce)
{
	// 非表示Stateでは強制表示の指定にかかわらず、通常の表示処理を行う
	UNREFERENCED_PARAMETER(isShowForce);
	auto context = GetContext();
	context->ShowWindowFromState();
	if (context->IsWindowVisibleFromState()) {
		// 再表示時に入力内容が残っている場合は検索中Stateへ復帰する
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
	// 非表示中に完了した検索結果も、ウインドウ側の既存処理へ反映する
	GetContext()->HandleQueryCompleted(result);
}
