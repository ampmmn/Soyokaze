
// BWLiteDlg.h : ヘッダー ファイル
//

#pragma once

#include <vector>
#include "KeywordEdit.h"
#include "IconLabel.h"

class CommandMap;
class Command;

class SharedHwnd;
class ExecHistory;
class HotKey;
class WindowPosition;
class WindowTransparency;

// CBWLiteDlg ダイアログ
class CBWLiteDlg : public CDialogEx
{
// コンストラクション
public:
	CBWLiteDlg(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~CBWLiteDlg();

// ダイアログ データ
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BWLITE_DIALOG };
#endif

public:
	static void ActivateWindow(HWND hwnd);
	void ActivateWindow();

	bool ExecuteCommand(const CString& commandStr);

	void OnContextMenu(CWnd* taskTrayWindow);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート

	CommandMap* GetCommandMap();
	void SetDescription(const CString& msg);

	// 現在選択中のコマンドを取得
	Command* GetCurrentCommand();


// 実装
protected:
	HICON m_hIcon;

	// キーワード入力欄の文字列
	CString mCommandStr;
	// 現在選択中のコマンドの説明
	CString m_strDescription;

	// 現在の候補
	std::vector<Command*> mCandidates;


	// コマンド管理マップ
	CommandMap* m_pCommandMap;
	// 選択中の候補
	int m_nSelIndex;

	// コマンド実行履歴
	ExecHistory* mExecHistory;

	// ウインドウハンドル(共有メモリに保存する用)
	SharedHwnd* m_pSharedHwnd;
	   // 後で起動したプロセスから有効化するために共有メモリに保存している

	// 候補一覧表示用リストボックス
	CListBox mCandidateListBox;
	// キーワード入力エディットボックス
	KeywordEdit mKeywordEdit;
	DWORD mLastCaretPos;

	// アイコン描画用ラベル
	IconLabel mIconLabel;
	//
	HotKey* mHotKeyPtr;
	// ウインドウ位置を保存するためのクラス
	WindowPosition* mWindowPositionPtr;
	// ウインドウの透明度を制御するためのクラス
	WindowTransparency* mWindowTransparencyPtr;

	// 生成された、メッセージ割り当て関数
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	virtual LRESULT WindowProc(UINT msg, WPARAM wp, LPARAM lp);

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEditCommandChanged();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnLbnSelChange();
	afx_msg void OnLbnDblClkCandidate();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnActivate(UINT, CWnd* wnd, BOOL bActive);
	LRESULT OnKeywordEditNotify(WPARAM wParam, LPARAM lParam);
	LRESULT OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

};
