#include "pch.h"
#include "mainwindow/state/MainWindowSearchingState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/MainWindowIdleState.h"
#include "mainwindow/state/MainWindowParamSearchingState.h"
#include "mainwindow/AppSound.h"

using namespace launcherapp::mainwindow::state;

SearchingState::SearchingState(LauncherWindowStateContextIF* context, bool allowParamSearching) :
	LauncherWindowStateBase(context),
	mAllowParamSearching(allowParamSearching)
{
}

void SearchingState::OnHideRequested()
{
	auto context = GetContext();
	context->ChangeState(std::make_unique<HiddenState>(context));
}

void SearchingState::OnExecuteRequested()
{
	auto context = GetContext();
	context->ExecuteCurrentCommand();
	if (context->IsWindowVisibleFromState() == false) {
		// コマンド実行後に終了動作でウインドウが閉じられた場合は状態を合わせる
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void SearchingState::OnCancel()
{
	auto context = GetContext();
	context->ClearContent();
	context->SetFocusToEdit();
	// 入力内容を消去したので、候補操作を行わない待機中Stateへ戻る
	context->ChangeState(std::make_unique<IdleState>(context));
}

void SearchingState::OnContentCleared()
{
	auto context = GetContext();
	if (context->IsWindowVisibleFromState()) {
		// 表示中に内容がクリアされた場合は待機中Stateへ戻る
		context->ChangeState(std::make_unique<IdleState>(context));
	}
	else {
		// 内容のクリアと同時にウインドウが閉じられた場合は非表示Stateへ戻る
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
}

void SearchingState::OnTextChanged()
{
	auto context = GetContext();
	// 確定直後だけ追加候補検索を抑制し、次の入力から通常動作へ戻す
	mAllowParamSearching = true;
	context->HandleTextChanged();
	if (context->HasKeyword() == false) {
		// 入力内容がなくなった場合は候補検索を行わない待機中Stateへ戻る
		context->ChangeState(std::make_unique<IdleState>(context));
	}
}

void SearchingState::OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result)
{
	auto context = GetContext();
	context->HandleQueryCompleted(result);
	if (context->IsWindowVisibleFromState() == false) {
		// 検索完了処理の結果、ウインドウが閉じられていれば非表示Stateへ遷移する
		context->ChangeState(std::make_unique<HiddenState>(context));
	}
	else if (context->HasKeyword() == false) {
		// 検索完了後に入力がなくなっていれば待機中Stateへ戻る
		context->ChangeState(std::make_unique<IdleState>(context));
	}
	else if (mAllowParamSearching && context->CanStartParamSearching()) {
		// 先行して通知された選択変更を処理してから追加候補Stateへ遷移する
		context->RequestParamSearching();
	}
}

bool SearchingState::OnKeyInput(unsigned int keyCode)
{
	auto context = GetContext();
	if (keyCode == VK_UP || keyCode == VK_DOWN) {
		if (context->IsCandidateListEmpty()) {
			// 候補がない場合は上下キーをStateで処理しない
			return false;
		}

		// 上下キーでは候補を循環させ、選択音と入力欄を更新する
		context->OffsetCandidateSelection(keyCode == VK_UP ? -1 : 1, true);
		AppSound::Get()->PlaySelectSound();
		context->UpdateCurrentCandidate();
		return true;
	}
	else if (keyCode == VK_TAB) {
		if (context->IsCandidateListEmpty()) {
			// Tabキーによるフォーカス移動を防ぐため、候補がなくても処理済みにする
			return true;
		}

		if (context->CanStartParamSearching()) {
			// パラメータ入力中はコマンド補完ではなく追加候補検索へ戻す
			context->RequestParamSearching();
			return true;
		}

		// コマンド名の入力中は現在の候補を使って入力内容を補完する
		context->Complement();
		return true;
	}
	else if (keyCode == VK_RETURN) {
		if (context->IsCandidateListEmpty()) {
			// 実行対象がない場合は通常のEnterキー処理へ委ねる
			return false;
		}
		OnExecuteRequested();
		return true;
	}
	else if (keyCode == VK_NEXT || keyCode == VK_PRIOR) {
		if (context->IsCandidateListEmpty()) {
			// 候補がない場合はページ移動を行わない
			return false;
		}

		// PageUp/PageDownでは、表示可能な件数単位で候補を移動する
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
