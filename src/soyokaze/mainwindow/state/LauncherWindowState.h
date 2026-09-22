#pragma once

namespace launcherapp { namespace mainwindow { namespace state {
class LauncherWindowStateContextIF;
}}}

namespace launcherapp { namespace commands { namespace core {
class CommandQueryResult;
}}}

namespace launcherapp { namespace mainwindow { namespace state {

/**
  ランチャーウインドウのStateが実装するイベントインターフェース
*/
class LauncherWindowState
{
public:
	virtual ~LauncherWindowState() = default;

	/** Stateへの遷移時に実行する処理 */
	virtual void OnEnter() = 0;
	/** Stateからの遷移時に実行する処理 */
	virtual void OnExit() = 0;
	/** ランチャーウインドウの表示要求を処理する */
	virtual void OnActivate(bool isShowForce) = 0;
	/** ランチャーウインドウの非表示要求を処理する */
	virtual void OnDeactivate() = 0;
	/** 現在の候補を実行する要求を処理する */
	virtual void OnExecuteRequested() = 0;
	/** キャンセル操作を処理する */
	virtual void OnCancel() = 0;
	/** 入力内容がクリアされたときの処理を行う */
	virtual void OnContentCleared() = 0;
	/** 入力内容が変更されたときの処理を行う */
	virtual void OnTextChanged() = 0;
	/** 非同期検索が完了したときの処理を行う */
	virtual void OnQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) = 0;
	/** 入力欄で押されたキーを処理する
	  @return true:キーを処理した  false:処理対象外
	*/
	virtual bool OnKeyInput(unsigned int keyCode) = 0;
	/** 候補の選択位置が変更されたときの処理を行う */
	virtual void OnCandidateSelectionChanged(int index) = 0;
	/** 候補がクリックされたときの処理を行う */
	virtual void OnCandidateClicked() = 0;
	/** 候補がダブルクリックされたときの処理を行う */
	virtual void OnCandidateDoubleClicked() = 0;
	/** キャレットや選択範囲が変更されたときの処理を行う */
	virtual void OnSelectionChanged() = 0;
	/** メインウインドウの位置・サイズ変更を処理する */
	virtual void OnWindowGeometryChanged() = 0;
	/** 追加候補欄の選択位置変更を処理する */
	virtual void OnExtraCandidateSelectionChanged() = 0;
	/** 追加候補欄のクリックを処理する */
	virtual void OnExtraCandidateClicked() = 0;
};

}}}
