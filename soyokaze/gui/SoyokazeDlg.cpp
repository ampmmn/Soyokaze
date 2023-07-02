
// SoyokazeDlg.cpp : 実装ファイル
//

#include "pch.h"
#include "framework.h"
#include "Soyokaze.h"
#include "gui/SoyokazeDlg.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "afxdialogex.h"
#include "utility/WindowPosition.h"
#include "SharedHwnd.h"
#include "ExecHistory.h"
#include "core/AppHotKey.h"
#include "WindowTransparency.h"
#include "IconLoader.h"
#include "AppPreference.h"
#include "utility/ProcessPath.h"
#include "utility/ScopeAttachThreadInput.h"
#include "core/CommandHotKeyManager.h"
#include <algorithm>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze;

struct CSoyokazeDlg::PImpl
{
	PImpl(CSoyokazeDlg* thisPtr) : 
		mDropTargetDialog(thisPtr),
		mDropTargetEdit(thisPtr)
	{
	}


	void ClearCandidates();

	HICON mIconHandle;

	// キーワード入力欄の文字列
	CString mCommandStr;
	// 現在選択中のコマンドの説明
	CString mDescriptionStr;

	// 現在の候補
	std::vector<soyokaze::core::Command*> mCandidates;

	// 選択中の候補
	int mSelIndex;

	// ウインドウハンドル(共有メモリに保存する用)
	SharedHwnd* mSharedHwnd;
	   // 後で起動したプロセスから有効化するために共有メモリに保存している

	// 候補一覧表示用リストボックス
	CListCtrl mCandidateListBox;
	// キーワード入力エディットボックス
	KeywordEdit mKeywordEdit;
	DWORD mLastCaretPos;

	// 外部からのコマンド受付用エディットボックス
	CmdReceiveEdit mCmdReceiveEdit;

	// アイコン描画用ラベル
	CaptureIconLabel mIconLabel;

	// 入力画面を呼び出すホットキー関連の処理をする
	AppHotKey* mHotKeyPtr;

	// ウインドウ位置を保存するためのクラス
	WindowPosition* mWindowPositionPtr;
	// ウインドウの透明度を制御するためのクラス
	WindowTransparency* mWindowTransparencyPtr;

	// ドロップターゲット
	SoyokazeDropTarget mDropTargetDialog;
	SoyokazeDropTarget mDropTargetEdit;

};


// 候補リストをクリアする
void CSoyokazeDlg::PImpl::ClearCandidates()
{
	for (auto command : mCandidates) {
		command->Release();
	}
	mCandidates.clear();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



// CSoyokazeDlg ダイアログ

CSoyokazeDlg::CSoyokazeDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SOYOKAZE_DIALOG, pParent),
	in(new PImpl(this))
{
	in->mHotKeyPtr = nullptr;
	in->mWindowPositionPtr= nullptr;
	in->mSharedHwnd = nullptr;
	in->mIconHandle = IconLoader::Get()->LoadDefaultIcon();
	in->mWindowTransparencyPtr = new WindowTransparency;
}

CSoyokazeDlg::~CSoyokazeDlg()
{
	in->ClearCandidates();

	delete in->mWindowTransparencyPtr;

	// 位置情報を設定ファイルに保存する
	delete in->mWindowPositionPtr;

	delete in->mHotKeyPtr;

	delete in->mSharedHwnd;

}

void CSoyokazeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, in->mCommandStr);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mDescriptionStr);
	DDX_Control(pDX, IDC_LIST_CANDIDATE, in->mCandidateListBox);
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
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_CANDIDATE, OnGetDispInfo)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_CANDIDATE, OnCandidatesCustomDraw)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_CANDIDATE, OnLvnItemChange)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_CANDIDATE, OnNMDblclk)
	ON_WM_SIZE()
	ON_MESSAGE(WM_APP+2, OnUserMessageActiveWindow)
	ON_MESSAGE(WM_APP+3, OnUserMessageSetText)
	ON_MESSAGE(WM_APP+4, OnUserMessageDragOverObject)
	ON_MESSAGE(WM_APP+5, OnUserMessageDropObject)
	ON_MESSAGE(WM_APP+6, OnUserMessageCaptureWindow)
	ON_MESSAGE(WM_APP+7, OnUserMessageHideAtFirst)
	ON_MESSAGE(WM_APP+8, OnUserMessageAppQuit)
	ON_WM_CONTEXTMENU()
	ON_WM_ENDSESSION()
	ON_COMMAND_RANGE(core::CommandHotKeyManager::ID_LOCAL_START, 
	                 core::CommandHotKeyManager::ID_LOCAL_END, OnCommandHotKey)
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
	TCHAR path[MAX_PATH_NTFS];
	GetModuleFileName(NULL, path, MAX_PATH_NTFS);
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


/**
 * ActiveWindow経由の処理
 * (後続プロセスから処理できるようにするためウインドウメッセージ経由で処理している)
 */
LRESULT CSoyokazeDlg::OnUserMessageActiveWindow(WPARAM wParam, LPARAM lParam)
{
	HWND hwnd = GetSafeHwnd();
	if (::IsWindowVisible(hwnd) == FALSE) {

		// 非表示状態なら表示
		ScopeAttachThreadInput scope;
		::ShowWindow(hwnd, SW_SHOW);
		::SetForegroundWindow(hwnd);
		::BringWindowToTop(hwnd);

		AppPreference* pref= AppPreference::Get();
		if (pref->IsIMEOffOnActive()) {
			in->mKeywordEdit.SetIMEOff();
		}
	}
	else {
		// 表示状態ではあるが、非アクティブならアクティブにする
		if (hwnd != ::GetActiveWindow()) {
			ScopeAttachThreadInput scope;
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
	else if (wnd == &in->mKeywordEdit) {
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

			TCHAR filePath[MAX_PATH_NTFS];
			for (int i = 0; i < fileCount; ++i) {
				DragQueryFile(dropInfo, i, filePath, MAX_PATH_NTFS);
				files.push_back(filePath);
			}
		}

		ASSERT(files.size() > 0);

		if (wnd == this) {
			// ファイル登録
			GetCommandRepository()->RegisterCommandFromFiles(files);
		}
		else if (wnd == &in->mKeywordEdit) {
			// キーワードのEdit欄にドロップされた場合はパスをコピー

			for (auto& str : files) {
				if (in->mCommandStr.IsEmpty() == FALSE) {
					in->mCommandStr += _T(" ");
				}
				in->mCommandStr += str;
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
				soyokaze::core::CommandParameter param;
				param.SetNamedParamString(_T("TYPE"), _T("ShellExecCommand"));
				param.SetNamedParamString(_T("PATH"), urlString);

				GetCommandRepository()->NewCommandDialog(&param);
			}
			else if (wnd == &in->mKeywordEdit) {
				in->mCommandStr += urlString;
				UpdateData(FALSE);
			}

			return 0;
		}
	}
	return 0;
}

LRESULT
CSoyokazeDlg::OnUserMessageCaptureWindow(WPARAM pParam, LPARAM lParam)
{
	HWND hTargetWnd = (HWND)lParam;
	if (IsWindow(hTargetWnd) == FALSE) {
		return 0;
	}

	ProcessPath processPath(hTargetWnd);

	// 自プロセスのウインドウなら何もしない
	if (GetCurrentProcessId() == processPath.GetProcessId()) {
		return 0;
	}

	// 
	try {
		soyokaze::core::CommandParameter param;
		param.SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param.SetNamedParamString(_T("COMMAND"), processPath.GetProcessName());
		param.SetNamedParamString(_T("PATH"), processPath.GetProcessPath());
		param.SetNamedParamString(_T("DESCRIPTION"), processPath.GetCaption());
		param.SetNamedParamString(_T("ARGUMENT"), processPath.GetCommandLine());

		GetCommandRepository()->NewCommandDialog(&param);
		return 0;
	}
	catch(ProcessPath::Exception& e) {
		CString errMsg((LPCTSTR)IDS_ERR_QUERYPROCESSINFO);
		CString pid;
		pid.Format(_T(" (PID:%d)"), e.GetPID());
		errMsg += pid;

		AfxMessageBox(errMsg);
		return 0;
	}
}


LRESULT CSoyokazeDlg::OnUserMessageHideAtFirst(
	WPARAM wParam,
	LPARAM lParam
)
{
	HideWindow();
	return 0;
}

LRESULT CSoyokazeDlg::OnUserMessageAppQuit(WPARAM wParam, LPARAM lParam)
{
	PostQuitMessage(0);
	return 0;
}

bool CSoyokazeDlg::ExecuteCommand(const CString& str)
{
	soyokaze::core::CommandParameter commandParam(str);

	auto cmd = GetCommandRepository()->QueryAsWholeMatch(commandParam.GetCommandString());
	if (cmd == nullptr) {
		return false;
	}

	std::thread th([cmd, str]() {
		soyokaze::core::CommandParameter commandParam(str);
		cmd->Execute(commandParam);
		cmd->Release();
	});
	th.detach();

	return true;
}

// CSoyokazeDlg メッセージ ハンドラー

BOOL CSoyokazeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();


	// グローバルホットキーのイベント受け取り先として登録する
	auto manager = core::CommandHotKeyManager::GetInstance();
	manager->SetReceiverWindow(GetSafeHwnd());

	in->mKeywordEdit.SubclassDlgItem(IDC_EDIT_COMMAND, this);
	in->mCmdReceiveEdit.SubclassDlgItem(IDC_EDIT_COMMAND2, this);
	in->mIconLabel.SubclassDlgItem(IDC_STATIC_ICON, this);

	in->mCandidateListBox.ModifyStyle(0, LVS_OWNERDATA);
	in->mCandidateListBox.SetExtendedStyle(in->mCandidateListBox.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// ヘッダー追加
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 300;
	lvc.fmt = LVCFMT_LEFT;
	in->mCandidateListBox.InsertColumn(0,&lvc);

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
	SetIcon(in->mIconHandle, TRUE);			// 大きいアイコンの設定
	SetIcon(in->mIconHandle, FALSE);		// 小さいアイコンの設定

	in->mSharedHwnd = new SharedHwnd(GetSafeHwnd());

	AppPreference* pref= AppPreference::Get();
	in->mDescriptionStr = pref->GetDefaultComment();

	// ウインドウ位置の復元
	in->mWindowPositionPtr = new WindowPosition();
	if (in->mWindowPositionPtr->Restore(GetSafeHwnd()) == false) {
		// 復元に失敗した場合は中央に表示
		CenterWindow();
	}

	// 透明度制御
	in->mWindowTransparencyPtr->SetWindowHandle(GetSafeHwnd());

	// 設定値の読み込み
	GetCommandRepository()->Load();

	// ホットキー登録
	in->mHotKeyPtr = new AppHotKey(GetSafeHwnd());
	if (in->mHotKeyPtr->Register() == false) {
		CString msg(_T("ホットキーを登録できませんでした。\n他のアプリケーションで使用されている可能性があります。\n"));
		msg += in->mHotKeyPtr->ToString();
		AfxMessageBox(msg);
	}
	
	UpdateData(FALSE);

	in->mDropTargetDialog.Register(this);
	in->mDropTargetEdit.Register(&in->mKeywordEdit);

	if (pref->IsHideOnStartup()) {
		PostMessage(WM_APP+7, 0, 0);
	}

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
		dc.DrawIcon(x, y, in->mIconHandle);
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
	return static_cast<HCURSOR>(in->mIconHandle);
}

CSoyokazeDlg::CommandRepository*
CSoyokazeDlg::GetCommandRepository()
{
	return CommandRepository::GetInstance();
}

void CSoyokazeDlg::SetDescription(const CString& msg)
{
	in->mDescriptionStr = msg;
	UpdateData(FALSE);
}

void CSoyokazeDlg::ClearContent()
{
	AppPreference* pref= AppPreference::Get();
	in->mDescriptionStr = pref->GetDefaultComment();
	in->mIconLabel.DrawDefaultIcon();
	in->mCommandStr.Empty();
	in->mCandidateListBox.DeleteAllItems();
	in->mSelIndex = -1;

	UpdateData(FALSE);
}


// 現在選択中のコマンドを取得
core::Command*
CSoyokazeDlg::GetCurrentCommand()
{
	if (in->mCandidates.empty()) {

		return nullptr;
	}

	if (in->mSelIndex < 0 || in->mCandidates.size() <= (size_t)in->mSelIndex) {
		return nullptr;
	}
	return in->mCandidates[in->mSelIndex];
}

/**
 * テキスト変更通知
 */
void CSoyokazeDlg::OnEditCommandChanged()
{
	DWORD curCaretPos = in->mKeywordEdit.GetSel();

	UpdateData();

	// キー入力でCtrl-Backspaceを入力したとき、不可視文字(0x7E→Backspace)が入力される
	// (Editコントロールの通常の挙動)
	// このアプリはCtrl-Backspaceで入力文字列を全クリアするが、一方で、上記挙動により
	// 入力文字列をクリアした後、0x7Eが挿入されるという謎挙動になるので、ここで0x7Fを明示的に消している
	if (in->mCommandStr.Find((TCHAR)0x7F) != -1) {
		TCHAR bsStr[] = { (TCHAR)0x7F, (TCHAR)0x00 };
		in->mCommandStr.Replace(bsStr, _T(""));
		in->mKeywordEdit.Clear();
	}

	in->mCandidateListBox.DeleteAllItems();
	in->mSelIndex = -1;

	// 入力テキストが空文字列の場合はデフォルト表示に戻す
	if (in->mCommandStr.IsEmpty()) {
		CString strMisMatch = AppPreference::Get()->GetDefaultComment();
		SetDescription(strMisMatch);
		in->mIconLabel.DrawDefaultIcon();
		return;
	}

	//
	soyokaze::core::CommandParameter commandParam(in->mCommandStr);

	// キーワードによる候補の列挙
	GetCommandRepository()->Query(commandParam.GetCommandString(), in->mCandidates);

	// 候補なし
	if (in->mCandidates.size() == 0) {
		CString strMisMatch;
		strMisMatch.LoadString(ID_STRING_MISMATCH);
		SetDescription(strMisMatch);
		in->mIconLabel.DrawIcon(IconLoader::Get()->LoadUnknownIcon());
		return;
	}

	// 候補リストの更新
	in->mCandidateListBox.SetItemCountEx((int)in->mCandidates.size());
	in->mSelIndex = 0;
	if (in->mSelIndex < in->mCandidates.size()) {
		in->mCandidateListBox.SetItemState(in->mSelIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		in->mSelIndex = -1;
	}

	// 候補先頭を選択状態にする
	auto pCmd = in->mCandidates[0];
	CString descriptionStr = pCmd->GetDescription();
	if (descriptionStr.IsEmpty()) {
		descriptionStr = pCmd->GetName();
	}
	SetDescription(descriptionStr);

	// サムネイルの更新
	in->mIconLabel.DrawIcon(pCmd->GetIcon());

	// 補完
	bool isCharAdded = (curCaretPos & 0xFFFF) > (in->mLastCaretPos & 0xFFFF);
	if (isCharAdded) {

		int start, end;
		in->mKeywordEdit.GetSel(start, end);
		if (commandParam.ComplementCommand(pCmd->GetName(), in->mCommandStr)) {
			// 補完が行われたらDDXにも反映し、入力欄の選択範囲も変える
			UpdateData(FALSE);
			in->mKeywordEdit.SetSel(end,-1);
		}
	}
	in->mLastCaretPos = in->mKeywordEdit.GetSel();

	in->mCandidateListBox.Invalidate();
}

void CSoyokazeDlg::OnOK()
{
	UpdateData();

	// コマンドを実行する
	auto cmd = GetCurrentCommand();
	if (cmd) {

		// 優先度を上げる
		GetCommandRepository()->AddRank(cmd, 10);

		CString str = in->mCommandStr;

		std::thread th([cmd, str]() {

			soyokaze::core::CommandParameter commandParam(str);

			// Ctrlキーが押されているかを設定
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				commandParam.SetNamedParamBool(_T("CtrlKeyPressed"), true);
			}

			if (cmd->Execute(commandParam) == FALSE) {
				AfxMessageBox(cmd->GetErrorString());
			}
		});
		th.detach();
	}
	else {
		// 空文字状態でEnterキーから実行したときはキーワードマネージャを表示
		if (in->mCommandStr.IsEmpty()) {
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
		if (in->mHotKeyPtr->IsSameKey(lp)) {
			// アプリ呼び出しホットキー
			ActivateWindow(GetSafeHwnd());
		}
		else {
			// コマンド実行ホットキー
			auto manager = core::CommandHotKeyManager::GetInstance();
			manager->InvokeGlobalHandler(lp);
		}
		return 0;
	}
	return CDialogEx::WindowProc(msg, wp, lp);
}

BOOL CSoyokazeDlg::PreTranslateMessage(MSG* pMsg)
{
	HACCEL accel = core::CommandHotKeyManager::GetInstance()->GetAccelerator();
	if (accel && TranslateAccelerator(GetSafeHwnd(), accel, pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}

void CSoyokazeDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	ClearContent();

	if (bShow) {
		GetDlgItem(IDC_EDIT_COMMAND)->SetFocus();
	}
	else {
		// 位置情報を更新する
		in->mWindowPositionPtr->Update(GetSafeHwnd());
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
	if (wParam == VK_BACK) {
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			ClearContent();
			in->mKeywordEdit.Clear();
			return 1;
		}
	}

	if (in->mCandidates.size() > 0) {
		if (wParam == VK_UP) {
			in->mSelIndex--;
			if (in->mSelIndex < 0) {
				in->mSelIndex = (int)(in->mCandidates.size()-1);

				in->mCandidateListBox.PostMessage(WM_KEYDOWN, VK_CONTROL, 0);
				in->mCandidateListBox.PostMessage(WM_KEYDOWN, VK_END, 0);
				in->mCandidateListBox.PostMessage(WM_KEYUP, VK_END, 0);
				in->mCandidateListBox.PostMessage(WM_KEYUP, VK_CONTROL, 0);
			}
			in->mCandidateListBox.SetItemState(in->mSelIndex, LVIS_SELECTED, LVIS_SELECTED);
			in->mCandidateListBox.EnsureVisible(in->mSelIndex, FALSE);

			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				return 1;
			}

			in->mCommandStr = cmd->GetName();
			in->mDescriptionStr = cmd->GetDescription();
			if (in->mDescriptionStr.IsEmpty()) {
				in->mDescriptionStr = cmd->GetName();
			}
			in->mIconLabel.DrawIcon(cmd->GetIcon());

			UpdateData(FALSE);

			in->mKeywordEdit.SetCaretToEnd();
			return 1;
		}
		else if (wParam ==VK_DOWN) {
			in->mSelIndex++;
			if (in->mSelIndex >= (int)in->mCandidates.size()) {
				in->mSelIndex = 0;
				in->mCandidateListBox.PostMessage(WM_KEYDOWN, VK_CONTROL, 0);
				in->mCandidateListBox.PostMessage(WM_KEYDOWN, VK_HOME, 0);
				in->mCandidateListBox.PostMessage(WM_KEYUP, VK_HOME, 0);
				in->mCandidateListBox.PostMessage(WM_KEYUP, VK_CONTROL, 0);
			}
			in->mCandidateListBox.SetItemState(in->mSelIndex, LVIS_SELECTED, LVIS_SELECTED);
			in->mCandidateListBox.EnsureVisible(in->mSelIndex, FALSE);

			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				return 1;
			}

			in->mCommandStr = cmd->GetName();
			in->mDescriptionStr = cmd->GetDescription();
			if (in->mDescriptionStr.IsEmpty()) {
				in->mDescriptionStr = cmd->GetName();
			}
			in->mIconLabel.DrawIcon(cmd->GetIcon());

			UpdateData(FALSE);

			in->mKeywordEdit.SetCaretToEnd();
			return 1;
		}
		else if (wParam == VK_TAB) {
			auto cmd = GetCurrentCommand();
			if (cmd == nullptr) {
				return 1;
			}

			in->mCommandStr = cmd->GetName();
			in->mCommandStr += _T(" ");

			in->mDescriptionStr = cmd->GetDescription();
			if (in->mDescriptionStr.IsEmpty()) {
				in->mDescriptionStr = cmd->GetName();
			}
			in->mIconLabel.DrawIcon(cmd->GetIcon());
			UpdateData(FALSE);

			in->mKeywordEdit.SetCaretToEnd();
			return 1;
		}
		else if (wParam == VK_NEXT || wParam == VK_PRIOR) {
			in->mCandidateListBox.PostMessage(WM_KEYDOWN, wParam, 0);
			return 1;
		}
	}

	return 0;
}

void CSoyokazeDlg::OnLbnSelChange()
{
	UpdateData();

	POSITION pos = in->mCandidateListBox.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	in->mSelIndex = in->mCandidateListBox.GetNextSelectedItem(pos);

	auto cmd = GetCurrentCommand();
	if (cmd) {
		in->mCommandStr = cmd->GetName();
		in->mDescriptionStr = cmd->GetDescription();
	}
	UpdateData(FALSE);
}

void CSoyokazeDlg::OnLbnDblClkCandidate()
{
	UpdateData();

	// コマンドを実行する
	auto cmd = GetCurrentCommand();
	if (cmd) {

		CString str = cmd->GetName();

		std::thread th([cmd,str]() {
			soyokaze::core::CommandParameter commandParam(str);
			if (cmd->Execute(commandParam) == FALSE) {
				AfxMessageBox(cmd->GetErrorString());
			}
		});
		th.detach();
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

void CSoyokazeDlg::OnGetDispInfo(
	NMHDR* pNMHDR,
	LRESULT* pResult
)
{
	*pResult = 0;

	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM* pItem = &(pDispInfo)->item;

	if (pItem->mask & LVIF_TEXT) {

		int itemIndex = pDispInfo->item.iItem;
		if (pDispInfo->item.iSubItem == 0) {
			if (0 <= itemIndex && itemIndex < in->mCandidates.size()) {
				auto cmd = in->mCandidates[itemIndex];
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetName(), _TRUNCATE);
			}
		}
	}
}

/**
 	リストコントロールのカスタムドロー処理
 	@param[in] pNMHDR  
 	@param[in] pResult 
*/
void CSoyokazeDlg::OnCandidatesCustomDraw(
	NMHDR* pNMHDR,
	LRESULT* pResult
)
{
	auto lpLvCd = (LPNMLVCUSTOMDRAW)pNMHDR;

	int drawStage = lpLvCd->nmcd.dwDrawStage;
	if (drawStage == CDDS_PREPAINT) {
		*pResult = CDRF_NOTIFYITEMDRAW;
		::SetWindowLong(GetSafeHwnd(), DWLP_MSGRESULT, (long)CDRF_NOTIFYITEMDRAW);
	}
	else if (drawStage == CDDS_ITEMPREPAINT) {
		*pResult = CDRF_NEWFONT;
		int row = (int)lpLvCd->nmcd.dwItemSpec;
		int state = in->mCandidateListBox.GetItemState(row, LVIS_SELECTED);

		if (state != 0) {
			// 選択状態のアイテムの背景色を変える
			lpLvCd->clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
			lpLvCd->clrText = RGB(0, 0, 0);
			::SetWindowLong(GetSafeHwnd(), DWLP_MSGRESULT, (long)CDRF_NEWFONT);
		}
	}
}

void CSoyokazeDlg::OnLvnItemChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	in->mSelIndex = nm->iItem;
	UpdateData(FALSE);

}

void CSoyokazeDlg::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	// ダブルクリックで確定
	OnOK();
}

void CSoyokazeDlg::OnSize(UINT type, int cx, int cy)
{
	__super::OnSize(type, cx, cy);

	if (in->mCandidateListBox.GetSafeHwnd()) {
		in->mCandidateListBox.SetColumnWidth(0, cx-70);
	}
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
	in->mWindowTransparencyPtr->UpdateActiveState(nState);
	__super::OnActivate(nState, wnd, bMinimized);
}

// Windowsの終了(ログオフ)通知
void CSoyokazeDlg::OnEndSession(BOOL isEnding)
{
	if (isEnding) {
		PostQuitMessage(0);
	}
}

void CSoyokazeDlg::OnCommandHotKey(UINT id)
{
	core::CommandHotKeyManager::GetInstance()->InvokeLocalHandler(id);
}
