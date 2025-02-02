
// LauncherMainWindow.h : ヘッダー ファイル
//

#pragma once

#include "mainwindow/LauncherMainWindowIF.h"

#include <vector>
#include <memory>
#include "tasktray/TaskTrayEventListenerIF.h"
#include "gui/KeywordEdit.h"
#include "mainwindow/CmdReceiveEdit.h"
#include "icon/CaptureIconLabel.h"
#include "mainwindow/LauncherDropTarget.h"


namespace launcherapp {
namespace core {
	class AppHotKey;
	class Command;
	class CommandHotKeyManager;
	class CommandRepository;
}
}

class SharedHwnd;
class WindowTransparency;

// LauncherMainWindow ダイアログ
class LauncherMainWindow :
 	public CDialogEx,
 	public TaskTrayEventListenerIF,
	public launcherapp::mainwindow::LauncherMainWindowIF
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
	void ClearContent();
	void Complement();
	void QueryAsync();
	void QuerySync();
	void UpdateCandidates();
	void WaitQueryRequest();
	void RunCommand(launcherapp::core::Command* cmd);
	void SelectCommandContextMenu(launcherapp::core::Command* cmd, int index);

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
	CStatic* GetGuideLabel() override;
	KeywordEdit* GetEdit() override;
	CandidateListCtrl* GetCandidateList() override;

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
	afx_msg void OnEndSession(BOOL isEnding);
	// コンテキストメニューの表示
	LRESULT OnKeywordEditNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnItemChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSizing(UINT side, LPRECT rect);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg void OnMove(int x, int y);
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
	LRESULT OnMessageSessionChange(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageCopyText(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageRequestCallback(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageClearContent(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnCommandHotKey(UINT id);
	afx_msg void OnCommandHelp();
	afx_msg void OnTimer(UINT_PTR timerId);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

};
