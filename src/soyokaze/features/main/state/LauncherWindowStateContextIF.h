#pragma once

#include <memory>

namespace launcherapp { namespace commands { namespace core {
class CommandQueryResult;
}}}

namespace launcherapp { namespace mainwindow { namespace state {

class LauncherWindowState;

/**
  ランチャーウインドウのStateから共通処理を利用するためのインターフェース
*/
class LauncherWindowStateContextIF
{
public:
	virtual ~LauncherWindowStateContextIF() {}

	/**
  次のStateへ遷移する
	@param[in] state 遷移先のState
*/
	virtual void ChangeState(std::unique_ptr<LauncherWindowState> state) = 0;

	/**
  ウインドウを表示し、アクティブ化する
*/
	virtual void ShowWindowFromState() = 0;

	/**
  表示中のウインドウをアクティブ化する
*/
	virtual void ActivateVisibleWindow() = 0;

	/**
  Stateからウインドウを非表示にする
*/
	virtual void HideWindowFromState() = 0;

	/**
  入力内容をクリアする
	*/
	virtual void ClearContent() = 0;

	/**
  入力欄へフォーカスを設定する
	*/
	virtual void SetFocusToEdit() = 0;

	/**
  入力変更時の既存処理を実行する
	*/
	virtual void HandleTextChanged() = 0;

	/**
	 入力欄の変更を内部状態へ反映する(候補検索は行わない)
	*/
	virtual void UpdateInputState() = 0;

	/**
  検索完了時の既存処理を実行する
	*/
	virtual void HandleQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) = 0;

	/**
  候補欄が空かどうかを確認する
	@return 候補欄が空の場合はtrue
*/
	virtual bool IsCandidateListEmpty() const = 0;

	/**
  候補の選択位置を移動する
	@param[in] offset 移動量
	@param[in] isLoop 末尾で先頭へ戻すかどうか
*/
	virtual void OffsetCandidateSelection(int offset, bool isLoop) = 0;

	/**
  現在の候補を入力欄へ反映する
*/
	virtual void UpdateCurrentCandidate() = 0;

	/**
  候補欄に表示できる項目数を取得する
	@return 1ページに表示できる項目数
*/
	virtual int GetCandidateCountInPage() = 0;

	/**
  入力内容を補完する
*/
	virtual void Complement() = 0;

	/**
  現在選択中の候補を入力欄などへ反映する
	*/
	virtual void ReflectCurrentCandidate() = 0;

	/**
  候補を選択する
	@param[in] index 選択する候補のインデックス
*/
	virtual void SelectCandidate(int index) = 0;

	/**
  現在選択中のコマンドを実行する
	*/
	virtual void ExecuteCurrentCommand() = 0;

	/**
  入力文字列が存在するか確認する
	@return 入力文字列が存在する場合はtrue
*/
	virtual bool HasKeyword() const = 0;

	/**
  ウインドウが表示されているか確認する
	@return 表示中の場合はtrue
*/
	virtual bool IsWindowVisibleFromState() const = 0;

	/**
  ウインドウがアクティブかどうかを確認する
  @return アクティブの場合はtrue
*/
	virtual bool IsWindowActive() const = 0;

	/**
  ウインドウのトグル表示設定が有効かどうかを確認する
  @return トグル表示が有効な場合はtrue
*/
	virtual bool IsShowToggleEnabled() const = 0;
	/** 現在の入力が追加候補検索を開始できるか確認する */
	virtual bool CanStartParamSearching() = 0;
	/** 追加候補検索への遷移をメッセージキューへ登録する */
	virtual void RequestParamSearching() = 0;
	/** 追加候補欄を更新して表示する */
	virtual void UpdateExtraCandidates() = 0;
	/** 追加候補欄を非表示にする */
	virtual void HideExtraCandidates() = 0;
	/** 追加候補の選択位置を移動する */
	virtual void OffsetExtraCandidateSelection(int offset) = 0;
	/** 追加候補欄が空か確認する */
	virtual bool IsExtraCandidateListEmpty() const = 0;
	/** 選択中の追加候補を入力欄へ反映する */
	virtual void ResolveExtraCandidate() = 0;
};

}}}
