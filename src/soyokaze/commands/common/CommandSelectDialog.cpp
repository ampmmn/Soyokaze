#include "pch.h"
#include "framework.h"
#include "commands/common/CommandSelectDialog.h"
#include "control/KeywordEdit.h"
#include "icon/IconLabel.h"
#include "commands/core/CommandRepository.h"
#include "matcher/PartialMatchPattern.h"
#include "utility/RefPtr.h"
#include "hotkey/CommandHotKeyManager.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr UINT TIMERID_FILTER = 1;

namespace launcherapp { namespace commands { namespace common {


using namespace launcherapp::core;

// リストの列情報
enum {
	COL_CMDNAME,      // コマンド名
	COL_CMDTYPE,      // 種別
	COL_DESCRIPTION,  // 説明
};

// ソート状態
enum {
	SORT_ASCEND_NAME,          // コマンド名-昇順
	SORT_DESCEND_NAME,         // コマンド名-降順
	SORT_ASCEND_DESCRIPTION,   // 説明-昇順
	SORT_DESCEND_DESCRIPTION,  // 説明-降順
	SORT_ASCEND_CMDTYPE,   // 種別-昇順
	SORT_DESCEND_CMDTYPE,  // 種別-降順
};


struct CommandSelectDialog::PImpl
{
	void SortCommands();
	void SelectItem(Command* command, bool isRedrawRequired);

	Command* GetItem(int index) {
		ASSERT(0 <= index && index < mShowCommands.size()); 
		return mShowCommands[index];
	}

	CString mName;
	CString mDescription;

	CString mFilterStr;

	std::vector<Command*> mCommands;
	Command* mSelCommand{nullptr};
	std::vector<Command*> mShowCommands;

	CListCtrl mListCtrl;
	std::unique_ptr<IconLabel> mIconLabelPtr;
	KeywordEdit mKeywordEdit;

	int mSortType{SORT_ASCEND_NAME};
	HWND mHwnd{nullptr};

	// フィルター更新タイマーID
	UINT_PTR mUpdateTimerId{0};
};

void CommandSelectDialog::PImpl::SortCommands()
{
	if (mSortType == SORT_ASCEND_NAME) {
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return l->GetName().CompareNoCase(r->GetName()) < 0;
		});
	}
	else if (mSortType == SORT_DESCEND_NAME) {
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return r->GetName().CompareNoCase(l->GetName()) < 0;
		});
	}
	else if (mSortType == SORT_ASCEND_DESCRIPTION) {
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return l->GetDescription() < r->GetDescription();
		});
	}
	else if (mSortType == SORT_DESCEND_DESCRIPTION) {
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return r->GetDescription() < l->GetDescription();
		});
	}
	else if (mSortType == SORT_ASCEND_CMDTYPE) {
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return l->GetTypeDisplayName() < r->GetTypeDisplayName();
		});
	}
	else if (mSortType == SORT_DESCEND_CMDTYPE) {
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return r->GetTypeDisplayName() < l->GetTypeDisplayName();
		});
	}
}

// 選択状態の更新
void CommandSelectDialog::PImpl::SelectItem(Command* command, bool isRedrawRequired)
{
	int selItemIndex = -1;

	int itemIndex = 0;
	for (auto& cmd : mShowCommands) {

		bool isSelItem = cmd == command;
		if (isSelItem) {
			selItemIndex = itemIndex;
		}
		mListCtrl.SetItemState(itemIndex, isSelItem ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
		itemIndex++;
	}
	if (selItemIndex != -1) {
		mListCtrl.EnsureVisible(selItemIndex, FALSE);
	}

	if (isRedrawRequired) {
		mListCtrl.Invalidate();
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CommandSelectDialog::CommandSelectDialog(CWnd* parent) : 
	CDialogEx(IDD_COMMANDSELECT, parent),
	in(std::make_unique<PImpl>())
{
	//SetHelpPageId("CommandSelect");

	in->mIconLabelPtr = std::make_unique<IconLabel>();
	in->mSortType = SORT_ASCEND_NAME;
}

CommandSelectDialog::~CommandSelectDialog()
{
	for (auto& cmd : in->mCommands) {
		cmd->Release();
	}
}

void CommandSelectDialog::SetCommandName(const CString& name)
{
	in->mName = name;
}

CString CommandSelectDialog::GetCommandName()
{
	return in->mName;
}


void CommandSelectDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_NAME, in->mName);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mDescription);
	DDX_Text(pDX, IDC_EDIT_FILTER, in->mFilterStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandSelectDialog, CDialogEx)
	ON_EN_CHANGE(IDC_EDIT_FILTER, OnEditFilterChanged)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMANDS, OnLvnItemChange)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COMMANDS, OnNMDblclk)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_COMMANDS, OnHeaderClicked)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_COMMANDS, OnGetDispInfo)
	ON_NOTIFY(LVN_ODFINDITEM , IDC_LIST_COMMANDS, OnFindCommand)
	ON_MESSAGE(WM_APP+1, OnUserMessageKeywrodEditKeyDown)
	ON_MESSAGE(WM_APP+2, OnUserMessageResetContent)
	ON_WM_TIMER()
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL CommandSelectDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mHwnd = GetSafeHwnd();

	SetIcon(IconLoader::Get()->LoadKeywordManagerIcon(), FALSE);

	in->mListCtrl.SubclassDlgItem(IDC_LIST_COMMANDS, this);
	in->mKeywordEdit.SubclassDlgItem(IDC_EDIT_FILTER, this);
	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabelPtr->DrawIcon(IconLoader::Get()->LoadKeywordManagerIcon());

	// フィルタ欄にプレースホルダーを設定する
	in->mKeywordEdit.SetPlaceHolder(_T("文字列をここに入力するとリストの絞り込みができます"));

	// リスト　スタイル変更
	in->mListCtrl.SetExtendedStyle(in->mListCtrl.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// ヘッダー追加
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	in->mListCtrl.InsertColumn(0,&lvc);

	strHeader.LoadString(IDS_COMMANDTYPE);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	in->mListCtrl.InsertColumn(1,&lvc);


	strHeader.LoadString(IDS_DESCRIPTION);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 150;
	lvc.fmt = LVCFMT_LEFT;
	in->mListCtrl.InsertColumn(2,&lvc);

	ResetContents();

	// 名前からコマンドを探す
	int commandCount = (int)in->mShowCommands.size();
	for (int i = 0; i < commandCount; ++i) {

		// コマンド名を小文字に変換したうえで前方一致比較をする
		CString item = in->mShowCommands[i]->GetName();
		if (item.CompareNoCase(in->mName) != 0) {
			continue;
		}
		// コマンドを選択状態にする
		in->SelectItem(in->mShowCommands[i], false);
		break;
	}

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void CommandSelectDialog::OnOK()
{
	if (in->mName.IsEmpty()) {
		return;
	}

	__super::OnOK();
}

void CommandSelectDialog::OnCancel()
{
	if (in->mUpdateTimerId != 0) {
		KillTimer(TIMERID_FILTER);
	}
	__super::OnCancel();
}

void CommandSelectDialog::ResetContents()
{
	// 以前のアイテムを解放
	for (auto& cmd : in->mCommands) {
		cmd->Release();
	}
	in->mCommands.clear();
	in->mShowCommands.clear();

	// コマンド一覧を取得する
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->EnumCommands(in->mCommands);

	// 現在のソート方法に従って要素をソート
	in->SortCommands();

	UpdateListItems();
}

bool CommandSelectDialog::UpdateStatus()
{
	POSITION pos = in->mListCtrl.GetFirstSelectedItemPosition();
	bool hasSelection = pos != nullptr;
	if (hasSelection) {
		int selItemIndex = in->mListCtrl.GetNextSelectedItem(pos);
		in->mSelCommand = in->mShowCommands[selItemIndex];
	}
	else {
		in->mSelCommand = nullptr;
	}

	GetDlgItem(IDOK)->EnableWindow(hasSelection);

	if (in->mSelCommand == nullptr) {
		in->mName.Empty();
		in->mDescription.Empty();
		return false;
	}

	CString name = in->mSelCommand->GetName();

	in->mIconLabelPtr->DrawIcon(in->mSelCommand->GetIcon());
	in->mName = name;
	in->mDescription = in->mSelCommand->GetDescription();

	return true;
}

void CommandSelectDialog::UpdateListItems()
{
	// フィルタ欄が空でない場合は絞り込みを行う
	if (in->mFilterStr.IsEmpty() == FALSE) {
		RefPtr<Pattern> pattern(PartialMatchPattern::Create());
		pattern->SetWholeText(in->mFilterStr);

		in->mShowCommands.clear();
		for (auto& cmd : in->mCommands) {

			auto name = cmd->GetName();
			auto desc = cmd->GetDescription();
			auto typeName = cmd->GetTypeDisplayName();
			
			if (pattern->Match(name) == Pattern::Mismatch &&
			    pattern->Match(desc) == Pattern::Mismatch &&
			    pattern->Match(typeName) == Pattern::Mismatch)  {
				continue;
			}
			in->mShowCommands.push_back(cmd);
		}
	}
	else {
		in->mShowCommands = in->mCommands;
	}

	ASSERT(in->mShowCommands.size() <= in->mCommands.size());

	// アイテム数を設定
	int visibleItems = (int)(in->mShowCommands.size());
	in->mListCtrl.SetItemCountEx(visibleItems);

	// 選択状態の更新
	in->SelectItem(in->mSelCommand, true);
}

void CommandSelectDialog::OnEditFilterChanged()
{
	// フィルター欄更新のつど更新するのではなく、
	// 最後のキー入力後の0.2秒後に更新を入れる

	if (in->mUpdateTimerId != 0) {
		KillTimer(TIMERID_FILTER);
	}
	in->mUpdateTimerId = SetTimer(TIMERID_FILTER, 200, 0);
}

void CommandSelectDialog::OnTimer(UINT_PTR timerId)
{
	if (timerId != TIMERID_FILTER) {
		return;
	}
	KillTimer(TIMERID_FILTER);
	in->mUpdateTimerId = 0;

	UpdateData();
	UpdateListItems();
	UpdateStatus();
}

/**
 *  リスト欄の要素の状態変更時の処理
 */
void CommandSelectDialog::OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	UpdateStatus();
	UpdateData(FALSE);
}

void CommandSelectDialog::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	OnOK();
}

/**
 *  リスト欄のヘッダクリック時の処理
 */
void CommandSelectDialog::OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;

	// クリックされた列で昇順/降順ソートをする

	int clickedCol = pNMLV->iSubItem;

	if(clickedCol == COL_CMDNAME) {
		// ソート方法の変更(コマンド名でソート)
		in->mSortType = in->mSortType == SORT_ASCEND_NAME ? SORT_DESCEND_NAME : SORT_ASCEND_NAME;
	}
	else if (clickedCol == COL_CMDTYPE) {
		// ソート方法の変更(説明でソート)
		in->mSortType = in->mSortType == SORT_ASCEND_CMDTYPE ? SORT_DESCEND_CMDTYPE : SORT_ASCEND_CMDTYPE;
	}
	else if (clickedCol == COL_DESCRIPTION) {
		// ソート方法の変更(説明でソート)
		in->mSortType = in->mSortType == SORT_ASCEND_DESCRIPTION ? SORT_DESCEND_DESCRIPTION : SORT_ASCEND_DESCRIPTION;
	}

	// ソート実施
	in->SortCommands();

	// 選択状態の更新
	UpdateListItems();
}

/**
 *  リスト欄のオーナーデータ周りの処理
 */
void CommandSelectDialog::OnGetDispInfo(
	NMHDR *pNMHDR,
	LRESULT *pResult
)
{
	*pResult = 0;

	NMLVDISPINFO* pDispInfo = (NMLVDISPINFO*)pNMHDR;
	LVITEM* pItem = &(pDispInfo)->item;

	if (pItem->mask & LVIF_TEXT) {

		int itemIndex = pDispInfo->item.iItem;
		if (pDispInfo->item.iSubItem == COL_CMDNAME) {
			// 1列目(コマンド名)のデータをコピー
			if (0 <= itemIndex && itemIndex < in->mCommands.size()) {
				auto cmd = in->GetItem(itemIndex);
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetName(), _TRUNCATE);
			}
		}
		else if (pDispInfo->item.iSubItem == COL_CMDTYPE) {
			// 説明列のデータをコピー
			if (0 <= itemIndex && itemIndex < in->mCommands.size()) {
				auto cmd = in->GetItem(itemIndex);
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetTypeDisplayName(), _TRUNCATE);
			}
		}
		else if (pDispInfo->item.iSubItem == COL_DESCRIPTION) {
			// 説明列のデータをコピー
			if (0 <= itemIndex && itemIndex < in->mCommands.size()) {
				auto cmd = in->GetItem(itemIndex);
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetDescription(), _TRUNCATE);
			}
		}
	}
}

/**
 *  オーナーデータリストの検索処理
 */
void CommandSelectDialog::OnFindCommand(
	NMHDR* pNMHDR,
	LRESULT* pResult
)
{
	NMLVFINDITEM* pFindInfo = (NMLVFINDITEM*)pNMHDR;

	if ((pFindInfo->lvfi.flags & LVFI_STRING) == 0) {
		*pResult = -1;
		return;
	}

	CString searchStr = pFindInfo->lvfi.psz;
	// 検索ワードを小文字に変換しておく
	searchStr.MakeLower();

	int startPos = pFindInfo->iStart;
	if (startPos >= in->mShowCommands.size()) {
		startPos = 0;
	}

	// 検索開始位置からリスト末尾までを探す
	int commandCount = (int)in->mShowCommands.size();
	for (int i = startPos; i < commandCount; ++i) {

		// コマンド名を小文字に変換したうえで前方一致比較をする
		CString item = in->mShowCommands[i]->GetName();
		item.MakeLower();
		if (item.Find(searchStr) == 0) {
			*pResult = i;
			return;
		}
	}
	// 末尾まで行ってヒットしなかった場合は先頭から検索開始位置までを探す
	for (int i = 0; i < startPos; ++i) {

		CString item = in->mShowCommands[i]->GetName();
		item.MakeLower();

		if (item.Find(searchStr) == 0) {
			*pResult = i;
			return;
		}
	}
}

LRESULT CommandSelectDialog::OnUserMessageKeywrodEditKeyDown(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// 矢印↓キー押下
	if (wParam ==VK_DOWN) {
		in->mListCtrl.SetFocus();

		if (in->mShowCommands.size() > 0) {
			in->mSelCommand = in->mShowCommands[0];
			in->SelectItem(in->mSelCommand, false);
		}
		return 1;
	}
	if (wParam ==VK_UP) {
		in->mListCtrl.SetFocus();

		if (in->mShowCommands.size() > 0) {
			int visibleItems = (int)(in->mShowCommands.size());
			in->mSelCommand = in->mShowCommands[visibleItems - 1];
			in->SelectItem(in->mSelCommand, false);
		}
		return 1;
	}
	else if (wParam == VK_TAB) {
		in->mListCtrl.SetFocus();
		return 1;
	}
	return 0;
}

LRESULT CommandSelectDialog::OnUserMessageResetContent(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	ResetContents();
	spdlog::info("KeywordManager content updated.");

	return 0;
}

}}}

