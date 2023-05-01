
// BWLiteDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "BWLite.h"
#include "BWLiteDlg.h"
#include "CommandMap.h"
#include "afxdialogex.h"
#include "WindowPlacementUtil.h"
#include "SharedHwnd.h"
#include "ExecHistory.h"
#include "HotKey.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CBWLiteDlg ダイアログ

CBWLiteDlg::CBWLiteDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_BWLITE_DIALOG, pParent),
	m_pCommandMap(new CommandMap()),
	m_pSharedHwnd(nullptr),
	mExecHistory(new ExecHistory),
	mHotKeyPtr(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
}

CBWLiteDlg::~CBWLiteDlg()
{
	mExecHistory->Save();
	delete mExecHistory;
	delete m_pCommandMap;
	delete m_pSharedHwnd;
	delete mHotKeyPtr;
}

void CBWLiteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, m_strCommand);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, m_strDescription);
	DDX_Control(pDX, IDC_LIST_CANDIDATE, mCandidateListBox);
}

BEGIN_MESSAGE_MAP(CBWLiteDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_COMMAND, OnEditCommandChanged)
	ON_WM_SHOWWINDOW()
	ON_MESSAGE(WM_APP+1, OnKeywordEditNotify)
END_MESSAGE_MAP()

void CBWLiteDlg::ActivateWindow(HWND hwnd)
{
	// 表示状態のトグル
	if (::IsWindowVisible(hwnd) == FALSE) {
		::ShowWindow(hwnd, SW_SHOW);
		::SetForegroundWindow(hwnd);
	}
	else {
		::ShowWindow(hwnd, SW_HIDE);
	}
}

void CBWLiteDlg::ActivateWindow()
{
	if (IsWindow(GetSafeHwnd())) {
		CBWLiteDlg::ActivateWindow(GetSafeHwnd());
	}
}

// CBWLiteDlg メッセージ ハンドラー

BOOL CBWLiteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	mKeywordEdit.SubclassDlgItem(IDC_EDIT_COMMAND, this);

	// "バージョン情報..." メニューをシステム メニューに追加します。

	// IDM_ABOUTBOX は、システム コマンドの範囲内になければなりません。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}


	// このダイアログのアイコンを設定します。アプリケーションのメイン ウィンドウがダイアログでない場合、
	//  Framework は、この設定を自動的に行います。
	SetIcon(m_hIcon, TRUE);			// 大きいアイコンの設定
	SetIcon(m_hIcon, FALSE);		// 小さいアイコンの設定

	mExecHistory->Load();
	m_pSharedHwnd = new SharedHwnd(GetSafeHwnd());

	m_strDescription.LoadString(ID_STRING_DEFAULTDESCRIPTION);

	// ウインドウ位置の復元
	util::window::LoadPlacement(this, _T("LauncherWindowPos"));

	// 設定値の読み込み
	m_pCommandMap->Load();

	// ホットキー登録
	mHotKeyPtr = new HotKey(GetSafeHwnd());
	if (mHotKeyPtr->Register() == false) {
		AfxMessageBox(_T("ホットキー登録できませんでした"));
	}
	
	UpdateData(FALSE);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CBWLiteDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 描画のデバイス コンテキスト

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// クライアントの四角形領域内の中央
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// アイコンの描画
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ユーザーが最小化したウィンドウをドラッグしているときに表示するカーソルを取得するために、
//  システムがこの関数を呼び出します。
HCURSOR CBWLiteDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CommandMap* CBWLiteDlg::GetCommandMap()
{
	return m_pCommandMap;
}

void CBWLiteDlg::SetDescription(const CString& msg)
{
	m_strDescription = msg;
	UpdateData(FALSE);
}

// 現在選択中のコマンドを取得
Command* CBWLiteDlg::GetCurrentCommand()
{
	if (mCandidates.empty()) {
		return nullptr;
	}

	if (m_nSelIndex < 0 || mCandidates.size() <= (size_t)m_nSelIndex) {
		return nullptr;
	}
	return mCandidates[m_nSelIndex];
}

void CBWLiteDlg::OnEditCommandChanged()
{
	UpdateData();

	mCandidateListBox.ResetContent();
	m_nSelIndex = -1;

	if (m_strCommand.IsEmpty()) {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_DEFAULTDESCRIPTION);
		SetDescription(strMisMatch);
		return;
	}

	GetCommandMap()->Query(m_strCommand, mCandidates);
	if (mCandidates.size() > 0) {

		// 履歴に基づきソート
		std::sort(mCandidates.begin(), mCandidates.end(), 
			[&](Command* l, Command* r) {
				size_t ageL = mExecHistory->GetOrder(l->GetName());
				size_t ageR = mExecHistory->GetOrder(r->GetName());
				return ageL < ageR;
		});

		auto pCmd = mCandidates[0];
		SetDescription(pCmd->GetDescription());

		for (auto& item : mCandidates) {
			mCandidateListBox.AddString(item->GetName());
		}
		m_nSelIndex = 0;
		mCandidateListBox.SetCurSel(m_nSelIndex);

	}
	else {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_MISMATCH);
		SetDescription(strMisMatch);
	}
}

void CBWLiteDlg::OnOK()
{
	// ToDo: コマンドを実行する
	auto cmd = GetCurrentCommand();
	if (cmd) {

		if (cmd->Execute() == FALSE) {
			AfxMessageBox(cmd->GetErrorString());
		}

		// コマンド実行履歴に追加
		mExecHistory->Add(m_strCommand);

	}

	ShowWindow(SW_HIDE);
}

void CBWLiteDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
}

LRESULT CBWLiteDlg::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_HOTKEY) {
		// ホットキー押下からの表示状態変更
		ActivateWindow(GetSafeHwnd());
		return 0;
	}
	return CDialogEx::WindowProc(msg, wp, lp);
}

void CBWLiteDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	if (bShow) {
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();
	}
	else {
		m_strDescription.LoadString(ID_STRING_DEFAULTDESCRIPTION);
		m_strCommand.Empty();
		mCandidateListBox.ResetContent();
		m_nSelIndex = -1;

		UpdateData(FALSE);
		util::window::SavePlacement(this, _T("LauncherWindowPos"));
	}
}


/**
 *
 */
LRESULT CBWLiteDlg::OnKeywordEditNotify(
	WPARAM wParam,
	LPARAM lParam
)
{
	if (mCandidates.size() > 0) {
		if (wParam == VK_UP) {
			m_nSelIndex--;
			if (m_nSelIndex < 0) {
				m_nSelIndex = (int)(mCandidates.size()-1);
			}
			mCandidateListBox.SetCurSel(m_nSelIndex);
			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				return 0;
			}

			m_strCommand = cmd->GetName();
			UpdateData(FALSE);

			mKeywordEdit.SetCaretToEnd();
			return 0;
		}
		else if (wParam ==VK_DOWN) {
			m_nSelIndex++;
			if (m_nSelIndex >= (int)mCandidates.size()) {
				m_nSelIndex = 0;
			}
			mCandidateListBox.SetCurSel(m_nSelIndex);
			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				return 0;
			}

			m_strCommand = cmd->GetName();
			UpdateData(FALSE);

			mKeywordEdit.SetCaretToEnd();
			return 0;
		}
		else if (wParam == VK_TAB) {
			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				return 0;
			}

			m_strCommand = cmd->GetName();
			UpdateData(FALSE);

			mKeywordEdit.SetCaretToEnd();
			return 0;
		}

	}
	return 0;
}
