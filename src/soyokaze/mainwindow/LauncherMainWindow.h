// LauncherMainWindow.h : ヘッダー ファイル
//

#pragma once

#include "mainwindow/LauncherMainWindowIF.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"

#include <vector>
#include <memory>
#include "tasktray/TaskTrayEventListenerIF.h"
#include "control/KeywordEdit.h"
#include "mainwindow/interprocess/CmdReceiveEdit.h"
#include "icon/CaptureIconLabel.h"
#include "mainwindow/guide/GuideCtrl.h"
#include "mainwindow/LauncherDropTarget.h"
#include "mainwindow/ExtraCandidateListCtrl.h"

class HOTKEY_ATTR;

namespace launcherapp {
namespace actions {
namespace core {
	class ParameterBuilder;
}
}

namespace core {
	class AppHotKey;
	class Command;
	class CommandHotKeyManager;
	class CommandRepository;
}
}

// LauncherMainWindow ダイアログ
class LauncherMainWindow :
 	public CDialogEx,
 	public TaskTrayEventListenerIF,
	public launcherapp::mainwindow::LauncherMainWindowIF,
	public launcherapp::mainwindow::state::LauncherWindowStateContextIF
{
	using CommandRepository = launcherapp::core::CommandRepository;
	using AppHotKey = launcherapp::core::AppHotKey;

// コンストラクション
public:
	LauncherMainWindow(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~LauncherMainWindow();

public:
	static void ActivateWindow(HWND hwnd);
	void ActivateWindow();
	void HideWindow();
	void ShowHelpTop();

	bool ExecuteCommand(const CString& commandStr);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

	CommandRepository* GetCommandRepository();
	void SetDescription(const CString& msg);
	/** 通常の入力内容クリア処理を実行する */
	void ClearContent() override;
	/** 入力内容をクリアし、必要に応じてレイアウトを強制更新する */
	void ClearContentImpl(bool isForceUpdate);
	void Complement() override;
	void QueryAsync();
	void QueryAsync(const CString& keyword);
	void QuerySync();
	void UpdateCandidates();
	void WaitQueryRequest();
	void RunCommand(launcherapp::core::Command* cmd);
	void RunCommand(launcherapp::core::Command* cmd, launcherapp::actions::core::ParameterBuilder* commandParam);
	void RunCommand(launcherapp::core::Command* cmd, launcherapp::actions::core::ParameterBuilder* commandParam, const HOTKEY_ATTR& hotkeyAttr);

	/** Stateからの表示要求を既存の表示処理へ委譲する */
	void ShowWindowFromState() override;
	/** 表示中のウインドウを前面へ移動する */
	void ActivateVisibleWindow() override;
	/** Stateからの非表示要求を既存の非表示処理へ委譲する */
	void HideWindowFromState() override;
	/** キーワード入力欄へフォーカスを設定する */
	void SetFocusToEdit() override;
	/** 入力変更に伴う既存処理を実行する */
	void HandleTextChanged() override;
	/** 入力欄の変更を内部状態へ反映する(候補検索は行わない) */
	void UpdateInputState() override;
	/** 検索結果を候補一覧へ反映する既存処理を実行する */
	void HandleQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override;
	/** 候補一覧が空かどうかを返す */
	bool IsCandidateListEmpty() const override;
	/** 候補の選択位置を移動する */
	void OffsetCandidateSelection(int offset, bool isLoop) override;
	/** 現在の候補を入力欄へ反映する */
	void UpdateCurrentCandidate() override;
	/** 候補一覧を1ページに表示できる件数を返す */
	int GetCandidateCountInPage() override;
	/** 現在の候補を入力欄などへ反映する */
	void ReflectCurrentCandidate() override;
	/** 候補の選択状態を更新する */
	void SelectCandidate(int index) override;
	/** 現在の候補を実行する */
	void ExecuteCurrentCommand() override;
	/** 入力欄にキーワードがあるかを返す */
	bool HasKeyword() const override;
	/** ウインドウが表示中かどうかを返す */
	bool IsWindowVisibleFromState() const override;
	/** ウインドウがアクティブかどうかを返す */
	bool IsWindowActive() const override;
	/** ウインドウ表示のトグル設定が有効かどうかを返す */
	bool IsShowToggleEnabled() const override;
	/** 現在のStateを終了し、指定されたStateへ遷移する */
	void ChangeState(std::unique_ptr<launcherapp::mainwindow::state::LauncherWindowState> state) override;
	bool CanStartParamSearching() override;
	void RequestParamSearching() override;
	void UpdateExtraCandidates() override;
	void HideExtraCandidates() override;
	void OffsetExtraCandidateSelection(int offset) override;
	bool IsExtraCandidateListEmpty() const override;
	void ResolveExtraCandidate() override;

	void SelectCommandContextMenu(launcherapp::core::Command* cmd, int index);
	void SetupCurrentCommandMenuItems(CMenu& menu, UINT menuIDFirst);

	// 現在選択中のコマンドを取得
	launcherapp::core::Command* GetCurrentCommand();

// 実装
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

	LRESULT OnTaskTrayLButtonDblclk() override;
	LRESULT OnTaskTrayContextMenu(CWnd* wnd, CPoint point) override;

// LauncherMainWindow
	CWnd* GetWindowObject() override;
	IconLabel* GetIconLabel() override;
	CStatic* GetDescriptionLabel() override;
	launcherapp::mainwindow::guide::GuideCtrl* GetGuideLabel() override;
	KeywordEdit* GetEdit() override;
	CandidateListCtrl* GetCandidateList() override;
	CFont* GetMainWindowFont() override;
	/** メインウインドウのフォント変更を追加候補Popupへ通知する */
	void OnMainWindowFontChanged(CFont* font) override;

	// 生成された、メッセージ割り当て関数
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp) override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEditCommandChanged();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnActivate(UINT, CWnd* wnd, BOOL bActive);
	afx_msg void OnEnterSizeMove();
	afx_msg void OnExitSizeMove();
	// コンテキストメニューの表示
	LRESULT OnKeywordEditNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnSelectionChangedMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageRequestParamSearching(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnItemChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSizing(UINT side, LPRECT rect);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnMove(int x, int y);
	afx_msg void OnMButtonUp(UINT flags, CPoint point);
	BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult) override;
	LRESULT OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageRunCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageSetText(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageSetSel(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageDragOverObject(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageDropObject(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageCaptureWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageHide(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageAppQuit(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageSetClipboardString(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageGetClipboardString(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageQueryComplete(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageBlockDeactivateOnUnfocus(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageUpdateCandidate(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageCopyText(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageRequestCallback(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageClearContent(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageMoveTemporary(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageBlockWindowDiaplay(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessagePopupMessage(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageExpandMacro(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageReleaseMacroStr(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageGuideClicked(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageDeleteWord(WPARAM wParam, LPARAM lParam);

	afx_msg void OnButtonOptionClicked();

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnCommandHotKey(UINT id);
	afx_msg void OnCommandHelp();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMeasureItem(int ctrlId, LPMEASUREITEMSTRUCT lpMeasureItemStruct);

};
