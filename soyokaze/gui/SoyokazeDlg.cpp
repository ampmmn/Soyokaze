
// SoyokazeDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "Soyokaze.h"
#include "gui/SoyokazeDlg.h"
#include "CommandRepository.h"
#include "CommandString.h"
#include "afxdialogex.h"
#include "utility/WindowPosition.h"
#include "SharedHwnd.h"
#include "ExecHistory.h"
#include "HotKey.h"
#include "WindowTransparency.h"
#include "IconLoader.h"
#include "AppPreference.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSoyokazeDlg ダイアログ

CSoyokazeDlg::CSoyokazeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SOYOKAZE_DIALOG, pParent),
	m_pCommandRepository(new CommandRepository()),
	m_pSharedHwnd(nullptr),
	mExecHistory(new ExecHistory),
	mHotKeyPtr(nullptr),
	mWindowPositionPtr(nullptr),
	mWindowTransparencyPtr(new WindowTransparency),
	mDropTargetDialog(this),
	mDropTargetEdit(this)
{
	m_hIcon = IconLoader::Get()->LoadDefaultIcon();
}

CSoyokazeDlg::~CSoyokazeDlg()
{
	mExecHistory->Save();

	delete mWindowTransparencyPtr;

	// 位置情報を設定ファイルに保存する
	delete mWindowPositionPtr;

	delete mHotKeyPtr;

	delete m_pSharedHwnd;

	delete mExecHistory;

	delete m_pCommandRepository;
}

void CSoyokazeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, mCommandStr);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, m_strDescription);
	DDX_Control(pDX, IDC_LIST_CANDIDATE, mCandidateListBox);
}

BEGIN_MESSAGE_MAP(CSoyokazeDlg, CDialogEx)
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
	ON_MESSAGE(WM_APP+4, OnUserMessageDragOverObject)
	ON_MESSAGE(WM_APP+5, OnUserMessageDropObject)
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CSoyokazeDlg::ActivateWindow(HWND hwnd)
{
	::PostMessage(hwnd, WM_APP+2, 0, 0);
}

void CSoyokazeDlg::ActivateWindow()
{
	if (IsWindow(GetSafeHwnd())) {
		CSoyokazeDlg::ActivateWindow(GetSafeHwnd());
	}
}

void CSoyokazeDlg::HideWindow()
{
	::ShowWindow(GetSafeHwnd(), SW_HIDE);
}


void CSoyokazeDlg::ShowHelp()
{
	TCHAR path[32768];
	GetModuleFileName(NULL, path, 32768);
	PathRemoveFileSpec(path);
	PathAppend(path, _T("help.html"));
	if (PathFileExists(path) == FALSE) {
		CString msg((LPCTSTR)IDS_ERR_HELPDOESNOTEXIST);
		msg += _T("\n");
		msg += path;
		AfxMessageBox(msg);
		return ;
	}

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_NORMAL;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;

	ShellExecuteEx(&si);
	CloseHandle(si.hProcess);
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

/**
 * ActiveWindow経由の処理
 * (後続プロセスから処理できるようにするためウインドウメッセージ経由で処理している)
 */
LRESULT CSoyokazeDlg::OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = GetSafeHwnd();
	if (::IsWindowVisible(hwnd) == FALSE) {

		// 非表示状態なら表示
		ScopeAttachThreadInput scope(GetWindowThreadProcessId(::GetForegroundWindow(),NULL));
		::ShowWindow(hwnd, SW_SHOW);
		::SetForegroundWindow(hwnd);
		::BringWindowToTop(hwnd);
	}
	else {
		// 表示状態ではあるが、非アクティブならアクティブにする
		if (hwnd != ::GetActiveWindow()) {
			ScopeAttachThreadInput scope(GetWindowThreadProcessId(::GetForegroundWindow(),NULL));
			::ShowWindow(hwnd, SW_SHOW);
			::SetForegroundWindow(hwnd);
			::BringWindowToTop(hwnd);
			return 0;
		}

		// 表示状態の場合はアプリ設定に応じて動作を変える

		AppPreference* pref= AppPreference::Get();
		if (pref->IsShowToggle()) {
			// トグル表示設定にしている場合は非表示にする
			::ShowWindow(hwnd, SW_HIDE);
		}
	}
	return 0;
}

/**
 * 後続プロセスから "-c <文字列>" 経由でコマンド実行指示を受け取ったときの処理
 */
LRESULT CSoyokazeDlg::OnUserMessageSetText(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR text = (LPCTSTR)lParam;
	if (text == nullptr) {
		return 0;
	}

	ExecuteCommand(text);
	return 0;
}


LRESULT 
CSoyokazeDlg::OnUserMessageDragOverObject(
	WPARAM wParam,
 	LPARAM lParam
)
{
	CWnd* wnd = (CWnd*)lParam;
	if (wnd == this) {
		SetDescription(CString((LPCTSTR)IDS_NEWREGISTER));
	}
	else if (wnd == &mKeywordEdit) {
		SetDescription(CString((LPCTSTR)IDS_PASTE));
	}
	return 0;
}

LRESULT 
CSoyokazeDlg::OnUserMessageDropObject(
	WPARAM wParam,
 	LPARAM lParam
)
{
	COleDataObject* dataObj = (COleDataObject*)wParam;
	CWnd* wnd = (CWnd*)lParam;

	if (dataObj->IsDataAvailable(CF_HDROP)) {
		std::vector<CString> files;

		STGMEDIUM st;
		if (dataObj->GetData(CF_HDROP, &st) ) {
			HDROP dropInfo = static_cast<HDROP>(st.hGlobal);

			int fileCount = (int)DragQueryFile( dropInfo, (UINT)-1, NULL, 0 );
			files.reserve(fileCount);

			TCHAR filePath[32768];
			for (int i = 0; i < fileCount; ++i) {
				DragQueryFile(dropInfo, i, filePath, 32768);
				files.push_back(filePath);
			}
		}

		ASSERT(files.size() > 0);

		if (wnd == this) {
			// ファイル登録
			GetCommandRepository()->RegisterCommandFromFiles(files);
		}
		else if (wnd == &mKeywordEdit) {

			for (auto& str : files) {
				if (mCommandStr.IsEmpty() == FALSE) {
					mCommandStr += _T(" ");
				}
				mCommandStr += str;
			}
			UpdateData(FALSE);
		}
		return 0;
	}

	UINT urlFormatId = RegisterClipboardFormat(CFSTR_INETURL);
	if (dataObj->IsDataAvailable(urlFormatId)) {

		STGMEDIUM st;
		if (dataObj->GetData(urlFormatId, &st) ) {
			CString urlString((LPCTSTR)GlobalLock(st.hGlobal));
			GlobalUnlock(st.hGlobal);

			if (wnd == this) {
				// URL登録
				GetCommandRepository()->NewCommandDialog(nullptr, &urlString);
			}
			else if (wnd == &mKeywordEdit) {
				mCommandStr += urlString;
				UpdateData(FALSE);
			}

			return 0;
		}
	}
	return 0;
}


bool CSoyokazeDlg::ExecuteCommand(const CString& str)
{
	CommandString commandStr(str);

	// (今のところ)内部用なので、履歴には登録しない
	auto cmd = GetCommandRepository()->QueryAsWholeMatch(commandStr.GetCommandString());
	if (cmd == nullptr) {
		return false;
	}

	std::vector<CString> args;
	commandStr.GetParameters(args);

	return cmd->Execute(args);
}

// CSoyokazeDlg メッセージ ハンドラー

BOOL CSoyokazeDlg::OnInitDialog()
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
	m_pCommandRepository->Load();

	// ホットキー登録
	mHotKeyPtr = new HotKey(GetSafeHwnd());
	if (mHotKeyPtr->Register() == false) {
		AfxMessageBox(_T("ホットキー登録できませんでした"));
	}
	
	UpdateData(FALSE);

	mDropTargetDialog.Register(this);
	mDropTargetEdit.Register(&mKeywordEdit);

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void CSoyokazeDlg::OnPaint()
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
HCURSOR CSoyokazeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

CommandRepository* CSoyokazeDlg::GetCommandRepository()
{
	return m_pCommandRepository;
}

void CSoyokazeDlg::SetDescription(const CString& msg)
{
	m_strDescription = msg;
	UpdateData(FALSE);
}

void CSoyokazeDlg::ClearContent()
{
	m_strDescription.LoadString(ID_STRING_DEFAULTDESCRIPTION);
	mIconLabel.DrawDefaultIcon();
	mCommandStr.Empty();
	mCandidateListBox.ResetContent();
	m_nSelIndex = -1;

	UpdateData(FALSE);
}


// 現在選択中のコマンドを取得
Command* CSoyokazeDlg::GetCurrentCommand()
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
void CSoyokazeDlg::OnEditCommandChanged()
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
	GetCommandRepository()->Query(commandStr.GetCommandString(), mCandidates);

	// 候補なし
	if (mCandidates.size() == 0) {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_MISMATCH);
		SetDescription(strMisMatch);
		mIconLabel.DrawIcon(IconLoader::Get()->LoadUnknownIcon());
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
	CString descriptionStr = pCmd->GetDescription();
	if (descriptionStr.IsEmpty()) {
		descriptionStr = pCmd->GetName();
	}
	SetDescription(descriptionStr);

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

void CSoyokazeDlg::OnOK()
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
	else {
		// 空文字状態でEnterキーから実行したときはキーワードマネージャを表示
		if (mCommandStr.IsEmpty()) {
			ExecuteCommand(_T("manager"));
		}
	}

	ShowWindow(SW_HIDE);
}

void CSoyokazeDlg::OnCancel()
{
	ShowWindow(SW_HIDE);
}

LRESULT CSoyokazeDlg::WindowProc(UINT msg, WPARAM wp, LPARAM lp)
{
	if (msg == WM_HOTKEY) {
		// ホットキー押下からの表示状態変更
		ActivateWindow(GetSafeHwnd());
		return 0;
	}
	return CDialogEx::WindowProc(msg, wp, lp);
}


void CSoyokazeDlg::OnShowWindow(BOOL bShow, UINT nStatus)
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
LRESULT CSoyokazeDlg::OnKeywordEditNotify(
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
			if (m_strDescription.IsEmpty()) {
				m_strDescription = cmd->GetName();
			}
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
			if (m_strDescription.IsEmpty()) {
				m_strDescription = cmd->GetName();
			}
			mIconLabel.DrawIcon(cmd->GetIcon());

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
			mCommandStr += _T(" ");

			m_strDescription = cmd->GetDescription();
			if (m_strDescription.IsEmpty()) {
				m_strDescription = cmd->GetName();
			}
			mIconLabel.DrawIcon(cmd->GetIcon());
			UpdateData(FALSE);

			mKeywordEdit.SetCaretToEnd();
			return 0;
		}

	}
	return 0;
}

void CSoyokazeDlg::OnLbnSelChange()
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

void CSoyokazeDlg::OnLbnDblClkCandidate()
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

// クライアント領域をドラッグしてウインドウを移動させるための処理
LRESULT CSoyokazeDlg::OnNcHitTest(
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

/**
 * コンテキストメニューの表示
 */
void CSoyokazeDlg::OnContextMenu(
	CWnd* pWnd,
	CPoint point
)
{
	CMenu menu;
	menu.CreatePopupMenu();

	const int ID_SHOW = 1;
	const int ID_HIDE = 2;
	const int ID_NEW = 3;
	const int ID_MANAGER = 4;
	const int ID_APPSETTING = 5;
	const int ID_USERDIR = 6;
	const int ID_MANUAL = 7;
	const int ID_VERSIONINFO = 8;
	const int ID_EXIT = 9;

	BOOL isVisible = IsWindowVisible();
	CString textToggleVisible(isVisible ? (LPCTSTR)IDS_MENUTEXT_HIDE : (LPCTSTR)IDS_MENUTEXT_SHOW);

	menu.InsertMenu(-1, 0, isVisible ? ID_HIDE : ID_SHOW, textToggleVisible);
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_APPSETTING, _T("アプリケーションの設定(&S)"));
	menu.InsertMenu(-1, 0, ID_NEW, _T("新規作成(&N)"));
	menu.InsertMenu(-1, 0, ID_MANAGER, _T("キーワードマネージャ(&K)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_USERDIR, _T("設定フォルダを開く(&D)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_MANUAL, _T("ヘルプ(&H)"));
	menu.InsertMenu(-1, 0, ID_VERSIONINFO, _T("バージョン情報(&V)"));
	menu.InsertMenu(-1, MF_SEPARATOR, 0, _T(""));
	menu.InsertMenu(-1, 0, ID_EXIT, _T("終了(&E)"));

	int n = menu.TrackPopupMenu(TPM_RETURNCMD, point.x, point.y, this);

	if (n == ID_SHOW) {
		ActivateWindow();
	}
	else if (n == ID_HIDE) {
		HideWindow();
	}
	else if (n == ID_NEW) {
		ExecuteCommand(_T("new"));
	}
	else if (n == ID_MANAGER) {
		ExecuteCommand(_T("manager"));
	}
	else if (n == ID_APPSETTING) {
		ExecuteCommand(_T("setting"));
	}
	else if (n == ID_USERDIR) {
		ExecuteCommand(_T("userdir"));
	}
	else if (n == ID_MANUAL) {
		// ヘルプ表示
		ShowHelp();
	}
	else if (n == ID_VERSIONINFO) {
		ExecuteCommand(_T("version"));
	}
	else if (n == ID_EXIT) {
		ExecuteCommand(_T("exit"));
	}
}

void CSoyokazeDlg::OnActivate(UINT nState, CWnd* wnd, BOOL bMinimized)
{
	mWindowTransparencyPtr->UpdateActiveState(nState);
	__super::OnActivate(nState, wnd, bMinimized);
}

