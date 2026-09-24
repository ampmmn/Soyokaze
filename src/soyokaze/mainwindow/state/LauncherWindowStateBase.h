#pragma once

#include "mainwindow/state/LauncherWindowState.h"

namespace launcherapp { namespace mainwindow { namespace state {

/**
  ランチャーウインドウStateの共通実装
  Contextの保持と、Stateごとの差分がないイベントの既定動作を提供する
*/
class LauncherWindowStateBase : public LauncherWindowState
{
public:
	/** 表示中Stateでの表示・再アクティブ化・トグル非表示を処理する */
	void OnActivate(bool isShowForce) override;

protected:
	/**
	  Stateから利用するContextを設定する
	  @param[in] context Stateが利用するContext
	*/
	explicit LauncherWindowStateBase(LauncherWindowStateContextIF* context);
	~LauncherWindowStateBase() override;

	/** StateからContextを取得する */
	LauncherWindowStateContextIF* GetContext() const;

	/** Stateへの遷移時に何もしない既定処理 */
	void OnEnter() override;
	/** Stateからの遷移時に何もしない既定処理 */
	void OnExit() override;
	/** 非表示要求を処理しないState向けの既定処理 */
	void OnDeactivate() override;
	/** 実行要求を処理しないState向けの既定処理 */
	void OnExecuteRequested() override;
	/** キャンセル操作を処理しないState向けの既定処理 */
	void OnCancel() override;
	/** 内容クリア通知を処理しないState向けの既定処理 */
	void OnContentCleared() override;
	/** 入力変更通知を処理しないState向けの既定処理 */
	void OnTextChanged() override;
	/** 検索完了通知を処理しないState向けの既定処理 */
	void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	/** キー入力を処理しないState向けの既定処理
	  @return false:キーを処理していない
	*/
	bool OnKeyInput(unsigned int keyCode) override;
	/** 候補選択変更を処理しないState向けの既定処理 */
	void OnCandidateSelectionChanged(int index) override;
	/** 候補クリックを処理しないState向けの既定処理 */
	void OnCandidateClicked() override;
	/** 候補ダブルクリックを処理しないState向けの既定処理 */
	void OnCandidateDoubleClicked() override;
	/** 入力欄のキャレットまたは選択範囲変更通知を処理しない既定動作 */
	void OnSelectionChanged() override;
	/** メインウインドウの位置またはサイズ変更通知を処理しない既定動作 */
	void OnWindowGeometryChanged() override;
	/** 追加候補一覧の選択位置変更通知を処理しない既定のイベント処理 */
	void OnExtraCandidateSelectionChanged() override;
	/** 追加候補一覧のクリック通知を処理しない既定動作 */
	void OnExtraCandidateClicked() override;

private:
	LauncherWindowStateContextIF* mContext;
};

}}}
