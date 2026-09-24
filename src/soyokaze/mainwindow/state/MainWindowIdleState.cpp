#include "pch.h"
#include "mainwindow/state/MainWindowIdleState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/MainWindowSearchingState.h"

using namespace launcherapp::mainwindow::state;

IdleState::IdleState(LauncherWindowStateContextIF* context) :
	LauncherWindowStateBase(context)
{
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
		// コマンド実行後に終了動作でウインドウが閉じられた場合は状態を合わせる
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void IdleState::OnCancel()
{
	auto context = GetContext();
	if (context->HasKeyword()) {
		// 入力中の内容がある場合は、まず内容だけをクリアして入力を継続する
		context->ClearContent();
		context->SetFocusToEdit();
	}
	else {
		// 入力内容がない場合のキャンセルは、ランチャーウインドウ自体を閉じる
		context->HideWindowFromState();
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void IdleState::OnTextChanged()
{
	auto context = GetContext();
	context->HandleTextChanged();
	if (context->HasKeyword()) {
		// 最初の文字が入力された時点で候補検索を扱うStateへ遷移する
		context->ChangeState(std::make_unique<SearchingState>(context));
	}
}

void IdleState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	auto context = GetContext();
	context->HandleQueryCompleted(result);
	if (context->IsWindowVisibleFromState() == false) {
		// 検索完了処理の結果、ウインドウが閉じられていれば非表示Stateへ遷移する
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
	else if (context->HasKeyword()) {
		// 検索完了後も入力が残っていれば候補操作を継続する
		context->ChangeState(std::make_unique<SearchingState>(context));
	}
}

bool IdleState::OnKeyInput(unsigned int keyCode)
{
	if (keyCode == VK_RETURN) {
		// 待機中のEnterキーは、候補の有無にかかわらず実行要求として扱う
		OnExecuteRequested();
		return true;
	}
	return false;
}
