
// SoyokazeDlg.h : ヘッダー ファイル
//

#pragma once

#include <vector>
#include <memory>
#include "gui/KeywordEdit.h"
#include "gui/CmdReceiveEdit.h"
#include "gui/CaptureIconLabel.h"
#include "gui/SoyokazeDropTarget.h"


namespace soyokaze {
namespace core {
	class AppHotKey;
	class Command;
	class CommandHotKeyManager;
	class CommandRepository;
}
}

class SharedHwnd;
class ExecHistory;
class WindowPosition;
class WindowTransparency;

// CSoyokazeDlg ダイアログ
class CSoyokazeDlg : public CDialogEx
{
	using CommandRepository = soyokaze::core::CommandRepository;
	using AppHotKey = soyokaze::core::AppHotKey;

// コンストラクション
public:
	CSoyokazeDlg(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~CSoyokazeDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SOYOKAZE_DIALOG };
#endif

public:
	static void ActivateWindow(HWND hwnd);
	void ActivateWindow();
	void HideWindow();
	void ShowHelp();

	bool ExecuteCommand(const CString& commandStr);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

	CommandRepository* GetCommandRepository();
	void SetDescription(const CString& msg);
	void ClearContent();

	// 現在選択中のコマンドを取得
	soyokaze::core::Command* GetCurrentCommand();

// 実装
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

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
	afx_msg void OnLbnSelChange();
	afx_msg void OnLbnDblClkCandidate();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnActivate(UINT, CWnd* wnd, BOOL bActive);
	afx_msg void OnEndSession(BOOL isEnding);
	// コンテキストメニューの表示
	LRESULT OnKeywordEditNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnItemChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT type, int cx, int cy);
	LRESULT OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageSetText(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageDragOverObject(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageDropObject(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageCaptureWindow(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageHideAtFirst(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageAppQuit(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageSetClipboardString(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageGetClipboardString(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnCommandHotKey(UINT id);
};
