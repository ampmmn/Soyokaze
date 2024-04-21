
// FilterDialog.cpp : 実装ファイル
//

#include "pch.h"
#include "FilterDialog.h"
#include "commands/filter/FilterExecutor.h"
#include "mainwindow/LauncherMainWindow.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "afxdialogex.h"
#include "utility/WindowPosition.h"
#include "SharedHwnd.h"
#include "hotkey/AppHotKey.h"
#include "setting/AppPreference.h"
#include "hotkey/CommandHotKeyManager.h"
#include "matcher/PartialMatchPattern.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace filter {


using namespace soyokaze;

struct FilterDialog::PImpl
{
	HICON mIconHandle;

	CString mCommandName;

	// 入力欄の文字列
	CString mInputStr;
	// 説明欄
	CString mInformationStr;

	FilterExecutor mExecutor;

	// 現在の候補
	std::vector<CString> mCandidates;

	// 選択中の候補
	int mSelIndex;

	// 候補一覧表示用リストボックス
	CListCtrl mCandidateListBox;
	// キーワード入力エディットボックス
	KeywordEdit mKeywordEdit;
	DWORD mLastCaretPos;

	// ウインドウ位置を保存するためのクラス
	std::unique_ptr<WindowPosition> mWindowPositionPtr;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



// FilterDialog ダイアログ

FilterDialog::FilterDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILTER, pParent),
	in(std::make_unique<PImpl>())
{
	in->mIconHandle = IconLoader::Get()->LoadDefaultIcon();
}

FilterDialog::~FilterDialog()
{
	// mWindowPositionPtrを破棄するときに位置情報を設定ファイルに保存する
}

void FilterDialog::SetCommandName(const CString& name)
{
	in->mCommandName = name;
}

void FilterDialog::SetText(const CString& text)
{
	int n = 0;

	in->mExecutor.ClearCandidates();

	CString token = text.Tokenize(_T("\n"), n);
	while(token.IsEmpty() == FALSE) {
		token.Trim(_T(" \t\r"));
		in->mExecutor.AddCandidates(token);
		token = text.Tokenize(_T("\n"), n);
	}
}

CString FilterDialog::GetFilteredText()
{
	if (in->mSelIndex < 0 || in->mCandidates.size() <= in->mSelIndex) {
		return _T("");
	}
	return in->mCandidates[in->mSelIndex];
}

void FilterDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_COMMAND, in->mInputStr);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mInformationStr);
	DDX_Control(pDX, IDC_LIST_CANDIDATE, in->mCandidateListBox);
}

void FilterDialog::UpdateStatus()
{
	in->mInformationStr.Format(_T("【%s】絞込み"), in->mCommandName);
	CString s;
	s.Format(_T("(%d/%d)"), in->mSelIndex + 1, (int)in->mCandidates.size());
	in->mInformationStr += s;
}

BEGIN_MESSAGE_MAP(FilterDialog, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT_COMMAND, OnEditCommandChanged)
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
//	ON_COMMAND_RANGE(core::CommandHotKeyManager::ID_LOCAL_START, 
//	                 core::CommandHotKeyManager::ID_LOCAL_END, OnCommandHotKey)
END_MESSAGE_MAP()


// FilterDialog メッセージ ハンドラー

BOOL FilterDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	in->mKeywordEdit.SubclassDlgItem(IDC_EDIT_COMMAND, this);

	in->mCandidateListBox.ModifyStyle(0, LVS_OWNERDATA);
	in->mCandidateListBox.SetExtendedStyle(in->mCandidateListBox.GetExtendedStyle()|LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

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

	in->mExecutor.Execute(_T(""), in->mCandidates);
	in->mCandidateListBox.SetItemCountEx((int)in->mCandidates.size());
	in->mSelIndex = 0;
	in->mCandidateListBox.SetItemState(in->mSelIndex, LVIS_SELECTED, LVIS_SELECTED);

	// ウインドウ位置の復元
	in->mWindowPositionPtr = std::make_unique<WindowPosition>(_T("filter"));
	if (in->mWindowPositionPtr->Restore(GetSafeHwnd()) == false) {
		// 復元に失敗した場合は中央に表示
		SetWindowPos(nullptr, 0, 0, 600, 300, SWP_NOZORDER|SWP_NOMOVE);
		CenterWindow();
	}

	UpdateStatus();
	UpdateData(FALSE);

	AppPreference* pref= AppPreference::Get();
	if (pref->IsHideOnStartup()) {
		PostMessage(WM_APP+7, 0, 0);
	}

	return TRUE;  // フォーカスをコントロールに設定した場合を除き、TRUE を返します。
}

// ダイアログに最小化ボタンを追加する場合、アイコンを描画するための
//  下のコードが必要です。ドキュメント/ビュー モデルを使う MFC アプリケーションの場合、
//  これは、Framework によって自動的に設定されます。

void FilterDialog::OnPaint()
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
HCURSOR FilterDialog::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(in->mIconHandle);
}

/**
 * テキスト変更通知
 */
void FilterDialog::OnEditCommandChanged()
{
	DWORD curCaretPos = in->mKeywordEdit.GetSel();

	UpdateData();

	// Ctrl-Backspace対応(※LauncherMainWindow::OnEditCommandChangedのコメント参照)
	if (in->mInputStr.Find((TCHAR)0x7F) != -1) {
		TCHAR bsStr[] = { (TCHAR)0x7F, (TCHAR)0x00 };
		in->mInputStr.Replace(bsStr, _T(""));
		in->mKeywordEdit.Clear();
	}

	// 候補を消す
	in->mCandidateListBox.DeleteAllItems();

	// 絞り込みの実施
	std::vector<CString> candidates;
	in->mExecutor.Execute(in->mInputStr, candidates);

	// 候補リストの更新
	in->mCandidateListBox.SetItemCountEx((int)candidates.size());
	in->mSelIndex = 0;
	if (in->mSelIndex < candidates.size()) {
		in->mCandidateListBox.SetItemState(in->mSelIndex, LVIS_SELECTED, LVIS_SELECTED);
	}
	else {
		in->mSelIndex = -1;
	}

	// 補完
	bool isCharAdded = (curCaretPos & 0xFFFF) > (in->mLastCaretPos & 0xFFFF);
	if (isCharAdded) {

//		int start, end;
//		in->mKeywordEdit.GetSel(start, end);
//		if (commandParam.ComplementCommand(pCmd->GetName(), in->mInputStr)) {
//			// 補完が行われたらDDXにも反映し、入力欄の選択範囲も変える
//			UpdateData(FALSE);
//			in->mKeywordEdit.SetSel(end,-1);
//		}
	}
	in->mLastCaretPos = in->mKeywordEdit.GetSel();
	in->mCandidates.swap(candidates);

	in->mCandidateListBox.Invalidate();

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterDialog::OnOK()
{
	UpdateData();
	// 何かあればここで処理
	in->mWindowPositionPtr->Update(GetSafeHwnd());
	in->mWindowPositionPtr.reset();

	__super::OnOK();
}

void FilterDialog::OnCancel()
{
	UpdateData();

	in->mWindowPositionPtr->Update(GetSafeHwnd());
	in->mWindowPositionPtr.reset();

	__super::OnCancel();
}

BOOL FilterDialog::PreTranslateMessage(MSG* pMsg)
{
//	HACCEL accel = core::CommandHotKeyManager::GetInstance()->GetAccelerator();
//	if (accel && TranslateAccelerator(GetSafeHwnd(), accel, pMsg)) {
//		return TRUE;
//	}
	return __super::PreTranslateMessage(pMsg);
}

/**
 *
 */
LRESULT FilterDialog::OnKeywordEditNotify(
	WPARAM wParam,
	LPARAM lParam
)
{
	// Ctrl-Backで全消去
	if (wParam == VK_BACK) {
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			in->mKeywordEdit.Clear();
			in->mInputStr.Empty();
			UpdateStatus();
			UpdateData(FALSE);
			return 1;
		}
	}

	// 候補がなければ何もしない
	if (in->mCandidates.empty()) {
		return 0;
	}

	// 上キー押下時は選択項目を一つ上に移動する
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
		in->mInputStr = in->mCandidates[in->mSelIndex];
		UpdateStatus();
		UpdateData(FALSE);

		in->mKeywordEdit.SetCaretToEnd();
		return 1;
	}
	// 下キー押下時は選択項目を一つ下に移動する
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
		in->mInputStr = in->mCandidates[in->mSelIndex];

		UpdateStatus();
		UpdateData(FALSE);
		in->mKeywordEdit.SetCaretToEnd();
		return 1;
	}
	// タブを押したら補完
	else if (wParam == VK_TAB) {
		in->mInputStr = in->mCandidates[in->mSelIndex];

		UpdateStatus();
		UpdateData(FALSE);
		in->mKeywordEdit.SetCaretToEnd();
		return 1;
	}
	// PageUp/PageDownでページ送り
	else if (wParam == VK_NEXT || wParam == VK_PRIOR) {
		in->mCandidateListBox.PostMessage(WM_KEYDOWN, wParam, 0);
	}

	return 0;
}

void FilterDialog::OnLbnSelChange()
{
	UpdateData();

	POSITION pos = in->mCandidateListBox.GetFirstSelectedItemPosition();
	if (pos == NULL) {
		return;
	}

	in->mSelIndex = in->mCandidateListBox.GetNextSelectedItem(pos);

	UpdateStatus();
	UpdateData(FALSE);
}

void FilterDialog::OnLbnDblClkCandidate()
{
	UpdateData();
	// ダブルクリックで確定
	OnOK();
}

// クライアント領域をドラッグしてウインドウを移動させるための処理
LRESULT FilterDialog::OnNcHitTest(
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

//void FilterDialog::OnCommandHotKey(UINT id)
//{
//	core::CommandHotKeyManager::GetInstance()->InvokeLocalHandler(id);
//}

void FilterDialog::OnGetDispInfo(
	NMHDR* pNMHDR,
	LRESULT* pResult
)
{
	*pResult = 0;

	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM* pItem = &(pDispInfo)->item;

	bool isText = pItem->mask & LVIF_TEXT;
	if (isText == false) {
		return;
	}

	if (pDispInfo->item.iSubItem != 0) {
		return;
	}

	size_t itemIndex = (size_t)pDispInfo->item.iItem;
	if (in->mCandidates.size() <= itemIndex) {
		return;
	}

	CListCtrl& listWnd = in->mCandidateListBox;

	int colWidth = listWnd.GetColumnWidth(0);

	int stringWidth = listWnd.GetStringWidth(in->mCandidates[itemIndex]);
	if (stringWidth > colWidth && stringWidth > 0) {
		// 部分文字列を表示
		CString str = in->mCandidates[itemIndex];

		double s = colWidth / (double)stringWidth;
		int n = (int)(str.GetLength() * s) - 2;
		if (n == 0) { n = 1; }

		str = str.Right(n);
		_tcsncpy_s(pItem->pszText, pItem->cchTextMax, str, _TRUNCATE);
	}
	else {
		// 全体を表示
		_tcsncpy_s(pItem->pszText, pItem->cchTextMax, in->mCandidates[itemIndex], _TRUNCATE);
	}
}

/**
 	リストコントロールのカスタムドロー処理
 	@param[in] pNMHDR  
 	@param[in] pResult 
*/
void FilterDialog::OnCandidatesCustomDraw(
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
			lpLvCd->clrText = RGB(255, 255, 255);
			::SetWindowLong(GetSafeHwnd(), DWLP_MSGRESULT, (long)CDRF_NEWFONT);
		}
		else {
			// 非選択状態のアイテムについては、交互に背景色を変える
			if (row % 2) {
				lpLvCd->clrTextBk = RGB(240, 240, 240);
				lpLvCd->clrText = RGB(0, 0, 0);
				::SetWindowLong(GetSafeHwnd(), DWLP_MSGRESULT, (long)CDRF_NEWFONT);
			}
		}
	}
}


void FilterDialog::OnLvnItemChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	in->mSelIndex = nm->iItem;
	UpdateStatus();
	UpdateData(FALSE);

}

void FilterDialog::OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult)
{
	*pResult = 0;
	 NMLISTVIEW* nm = (NMLISTVIEW*)pNMHDR;
	// ダブルクリックで確定
	OnOK();
}

void FilterDialog::OnSize(UINT type, int cx, int cy)
{
	__super::OnSize(type, cx, cy);

	if (in->mCandidateListBox.GetSafeHwnd()) {
		in->mCandidateListBox.SetColumnWidth(0, cx-30);
	}
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze


