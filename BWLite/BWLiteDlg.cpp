
// BWLiteDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "BWLite.h"
#include "BWLiteDlg.h"
#include "CommandMap.h"
#include "CommandString.h"
#include "afxdialogex.h"
#include "WindowPlacementUtil.h"
#include "SharedHwnd.h"
#include "ExecHistory.h"
#include "HotKey.h"
#include "WindowTransparency.h"
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
	mHotKeyPtr(nullptr),
	mWindowPositionPtr(nullptr),
	mWindowTransparencyPtr(new WindowTransparency)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON2);
}

CBWLiteDlg::~CBWLiteDlg()
{
	mExecHistory->Save();

	delete mWindowTransparencyPtr;

	// 位置情報を設定ファイルに保存する
	delete mWindowPositionPtr;

	delete mHotKeyPtr;

	delete m_pSharedHwnd;

	delete mExecHistory;

	delete m_pCommandMap;
}

void CBWLiteDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, mCommandStr);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, m_strDescription);
	DDX_Control(pDX, IDC_LIST_CANDIDATE, mCandidateListBox);
}

BEGIN_MESSAGE_MAP(CBWLiteDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_COMMAND, OnEditCommandChanged)
	ON_WM_SHOWWINDOW()
	ON_LBN_SELCHANGE(IDC_LIST_CANDIDATE, OnLbnSelChange)
	ON_LBN_DBLCLK(IDC_LIST_CANDIDATE, OnLbnDblClkCandidate)
	ON_WM_NCHITTEST()
	ON_WM_ACTIVATE()
	ON_MESSAGE(WM_APP+1, OnKeywordEditNotify)
	ON_MESSAGE(WM_APP+2, OnUserMessageActiveWindow)
	ON_MESSAGE(WM_APP+3, OnUserMessageSetText)
END_MESSAGE_MAP()

void CBWLiteDlg::ActivateWindow(HWND hwnd)
{
	::PostMessage(hwnd, WM_APP+2, 0, 0);
}

void CBWLiteDlg::ActivateWindow()
{
	if (IsWindow(GetSafeHwnd())) {
		CBWLiteDlg::ActivateWindow(GetSafeHwnd());
	}
}


class ScopeAttachThreadInput
{
public:
	ScopeAttachThreadInput(DWORD target) : target(target) {
		AttachThreadInput(GetCurrentThreadId(), target, TRUE);
	}
	~ScopeAttachThreadInput()
	{
		AttachThreadInput(GetCurrentThreadId(), target, FALSE);
	}
	DWORD target;
};

LRESULT CBWLiteDlg::OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam)
{
	// 表示状態のトグル
	HWND hwnd = GetSafeHwnd();
	if (::IsWindowVisible(hwnd) == FALSE) {

		ScopeAttachThreadInput scope(GetWindowThreadProcessId(::GetForegroundWindow(),NULL));
		::ShowWindow(hwnd, SW_SHOW);
		::SetForegroundWindow(hwnd);
		::BringWindowToTop(hwnd);
	}
	else {
		// 非アクティブならアクティブにする
		if (hwnd != ::GetActiveWindow()) {
			ScopeAttachThreadInput scope(GetWindowThreadProcessId(::GetForegroundWindow(),NULL));
			::ShowWindow(hwnd, SW_SHOW);
			::SetForegroundWindow(hwnd);
			::BringWindowToTop(hwnd);
		}
		else {
			::ShowWindow(hwnd, SW_HIDE);
		}
	}
	return 0;
}

/**
 * 後続プロセスから "-c <文字列>" 経由でコマンド実行指示を受け取ったときの処理
 */
LRESULT CBWLiteDlg::OnUserMessageSetText(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR text = (LPCTSTR)lParam;
	if (text == nullptr) {
		return 0;
	}

	ExecuteCommand(text);
	return 0;
}

bool CBWLiteDlg::ExecuteCommand(const CString& commandStr)
{
	// (今のところ)内部用なので、履歴には登録しない
	auto cmd = GetCommandMap()->QueryAsWholeMatch(commandStr);
	if (cmd == nullptr) {
		return false;
	}
	return cmd->Execute();
}

// CBWLiteDlg メッセージ ハンドラー

BOOL CBWLiteDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	mKeywordEdit.SubclassDlgItem(IDC_EDIT_COMMAND, this);
	mCmdReceiveEdit.SubclassDlgItem(IDC_EDIT_COMMAND2, this);
	mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);


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
	mWindowPositionPtr = new WindowPosition();
	if (mWindowPositionPtr->Restore(GetSafeHwnd()) == false) {
		// 復元に失敗した場合は中央に表示
		CenterWindow();
	}

	// 透明度制御
	mWindowTransparencyPtr->SetWindowHandle(GetSafeHwnd());

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

void CBWLiteDlg::ClearContent()
{
	m_strDescription.LoadString(ID_STRING_DEFAULTDESCRIPTION);
	mIconLabel.DrawDefaultIcon();
	mCommandStr.Empty();
	mCandidateListBox.ResetContent();
	m_nSelIndex = -1;

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

/**
 * テキスト変更通知
 */
void CBWLiteDlg::OnEditCommandChanged()
{
	DWORD curCaretPos = mKeywordEdit.GetSel();

	UpdateData();


	mCandidateListBox.ResetContent();
	m_nSelIndex = -1;

	// 入力テキストが空文字列の場合はデフォルト表示に戻す
	if (mCommandStr.IsEmpty()) {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_DEFAULTDESCRIPTION);
		SetDescription(strMisMatch);
		mIconLabel.DrawDefaultIcon();
		return;
	}

	//
	CommandString commandStr(mCommandStr);

	// キーワードによる候補の列挙
	GetCommandMap()->Query(commandStr.GetCommandString(), mCandidates);

	// 候補なし
	if (mCandidates.size() == 0) {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_MISMATCH);
		SetDescription(strMisMatch);
		mIconLabel.DrawDefaultIcon();
		return;
	}

	// 履歴に基づきソート
	std::sort(mCandidates.begin(), mCandidates.end(),
			[&](Command* l, Command* r) {
			size_t ageL = mExecHistory->GetOrder(l->GetName());
			size_t ageR = mExecHistory->GetOrder(r->GetName());
			return ageL < ageR;
			});

	// 候補リストの更新
	for (auto& item : mCandidates) {
		mCandidateListBox.AddString(item->GetName());
	}
	m_nSelIndex = 0;
	mCandidateListBox.SetCurSel(m_nSelIndex);

	// 候補先頭を選択状態にする
	auto pCmd = mCandidates[0];
	SetDescription(pCmd->GetDescription());

	// サムネイルの更新
	mIconLabel.DrawIcon(pCmd->GetIcon());

	// 補完
	bool isCharAdded = (curCaretPos & 0xFFFF) > (mLastCaretPos & 0xFFFF);
	if (isCharAdded) {

		int start, end;
		mKeywordEdit.GetSel(start, end);
		if (commandStr.ComplementCommand(pCmd->GetName(), mCommandStr)) {
			// 補完が行われたらDDXにも反映し、入力欄の選択範囲も変える
			UpdateData(FALSE);
			mKeywordEdit.SetSel(end,-1);
		}
	}
	mLastCaretPos = mKeywordEdit.GetSel();
}

void CBWLiteDlg::OnOK()
{
	UpdateData();

	// コマンドを実行する
	auto cmd = GetCurrentCommand();
	if (cmd) {

		CommandString commandStr(mCommandStr);

		std::vector<CString> args;
		commandStr.GetParameters(args);

		if (cmd->Execute(args) == FALSE) {
			AfxMessageBox(cmd->GetErrorString());
		}

		// コマンド実行履歴に追加
		mExecHistory->Add(mCommandStr);

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
	ClearContent();

	if (bShow) {
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();
	}
	else {
		// 位置情報を更新する
		mWindowPositionPtr->Update(GetSafeHwnd());
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

			mCommandStr = cmd->GetName();
			m_strDescription = cmd->GetDescription();
			mIconLabel.DrawIcon(cmd->GetIcon());

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

			mCommandStr = cmd->GetName();
			m_strDescription = cmd->GetDescription();
			UpdateData(FALSE);

			mKeywordEdit.SetCaretToEnd();
			return 0;
		}
		else if (wParam == VK_TAB) {
			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				return 0;
			}

			mCommandStr = cmd->GetName();
			m_strDescription = cmd->GetDescription();
			mIconLabel.DrawIcon(cmd->GetIcon());
			UpdateData(FALSE);

			mKeywordEdit.SetCaretToEnd();
			return 0;
		}

	}
	return 0;
}

void CBWLiteDlg::OnLbnSelChange()
{
	UpdateData();

	int nSel = mCandidateListBox.GetCurSel();
	if (nSel == -1) {
		return;
	}
	m_nSelIndex = nSel;
	auto cmd = GetCurrentCommand();
	if (cmd) {
		mCommandStr = cmd->GetName();
		m_strDescription = cmd->GetDescription();
	}
	UpdateData(FALSE);
}

void CBWLiteDlg::OnLbnDblClkCandidate()
{
	OnOK();
}

// クライアント領域をドラッグしてウインドウを移動させるための処理
LRESULT CBWLiteDlg::OnNcHitTest(
	CPoint point
)
{

	RECT rect;
	GetClientRect (&rect);

	CPoint ptClient(point);
	ScreenToClient(&ptClient);

	if (PtInRect(&rect, ptClient) && (GetAsyncKeyState( VK_LBUTTON ) & 0x8000) )
	{
		return HTCAPTION;
	}
	return __super::OnNcHitTest(point);
}

void CBWLiteDlg::OnContextMenu(
	CWnd* taskTrayWindow
)
{
	CPoint point;
	::GetCursorPos(&point);

	CMenu menu;
	menu.CreatePopupMenu();

	const int ID_SHOW = 1;
	const int ID_APPSETTING = 2;
	const int ID_USERDIR = 3;
	const int ID_VERSIONINFO = 4;
	const int ID_EXIT = 5;

	CString textShow((LPCTSTR)IDS_MENUTEXT_SHOW);
	menu.InsertMenu(-1, 0, ID_SHOW, textShow);
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	//
	menu.InsertMenu(-1, 0, ID_APPSETTING, _T("アプリケーションの設定(&S)"));
	menu.InsertMenu(-1, 0, ID_USERDIR, _T("設定フォルダを開く(&D)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_VERSIONINFO, _T("バージョン情報(&V)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_EXIT, _T("終了(&E)"));

	int n = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);

	if (n == ID_SHOW) {
		ActivateWindow();
	}
	else if (n == ID_APPSETTING) {
		//ExecuteCommand(_T("setting"));
	}
	else if (n == ID_USERDIR) {
		ExecuteCommand(_T("userdir"));
	}
	else if (n == ID_VERSIONINFO) {
		ExecuteCommand(_T("version"));
	}
	else if (n == ID_EXIT) {
		ExecuteCommand(_T("exit"));
	}
}

void CBWLiteDlg::OnActivate(UINT nState, CWnd* wnd, BOOL bMinimized)
{
	mWindowTransparencyPtr->UpdateActiveState(nState);
	__super::OnActivate(nState, wnd, bMinimized);
}
