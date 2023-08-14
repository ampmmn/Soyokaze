#include "pch.h"
#include "framework.h"
#include "gui/KeywordManagerDialog.h"
#include "gui/IconLabel.h"
#include "core/CommandRepository.h"
#include "utility/TopMostMask.h"
#include "IconLoader.h"
#include "resource.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Command = soyokaze::core::Command;

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


struct KeywordManagerDialog::PImpl
{
	void SortCommands();

	CString mName;
	CString mDescription;

	std::vector<Command*> mCommands;
	Command* mSelCommand;

	CListCtrl mListCtrl;
	std::unique_ptr<IconLabel> mIconLabelPtr;

	TopMostMask mTopMostMask;
	int mSortType;
};

void KeywordManagerDialog::PImpl::SortCommands()
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


KeywordManagerDialog::KeywordManagerDialog() : 
	CDialogEx(IDD_KEYWORDMANAGER),
	in(new PImpl)
{
	in->mIconLabelPtr.reset(new IconLabel());
	in->mSortType = SORT_ASCEND_NAME;
	in->mSelCommand = nullptr;
}

KeywordManagerDialog::~KeywordManagerDialog()
{
	for (auto cmd : in->mCommands) {
		cmd->Release();
	}
}

void KeywordManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_NAME, in->mName);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mDescription);
}

BEGIN_MESSAGE_MAP(KeywordManagerDialog, CDialogEx)
	ON_COMMAND(IDC_BUTTON_NEW, OnButtonNew)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMANDS, OnLvnItemChange)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COMMANDS, OnNMDblclk)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_COMMANDS, OnHeaderClicked)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_COMMANDS, OnGetDispInfo)
	ON_NOTIFY(LVN_ODFINDITEM , IDC_LIST_COMMANDS, OnFindCommand)
END_MESSAGE_MAP()


BOOL KeywordManagerDialog::OnInitDialog()
{
	__super::OnInitDialog();

	SetIcon(IconLoader::Get()->LoadDefaultIcon(), FALSE);

	in->mListCtrl.SubclassDlgItem(IDC_LIST_COMMANDS, this);
	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);

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
	lvc.cx = 200;
	lvc.fmt = LVCFMT_LEFT;
	in->mListCtrl.InsertColumn(2,&lvc);

	ResetContents();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

void KeywordManagerDialog::ResetContents()
{
	// 更新前の選択位置を覚えておく
	int selItemIndex = -1;
	POSITION pos = in->mListCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		selItemIndex = in->mListCtrl.GetNextSelectedItem(pos);
	}

	// 以前のアイテムを解放
	for (auto cmd : in->mCommands) {
		cmd->Release();
	}
	in->mCommands.clear();

	// コマンド一覧を取得する
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	cmdRepoPtr->EnumCommands(in->mCommands);
	
	// 現在のソート方法に従って要素をソート
	in->SortCommands();


	// アイテム数を設定
	in->mListCtrl.SetItemCountEx((int)in->mCommands.size());

	// 選択状態の更新
	int itemIndex = 0;
	for (auto cmd : in->mCommands) {
		bool isSelItem = itemIndex== selItemIndex;
		if (isSelItem) {
			in->mSelCommand = cmd;
		}
		in->mListCtrl.SetItemState(itemIndex, isSelItem ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
		itemIndex++;
	}
	in->mListCtrl.Invalidate();
}

bool KeywordManagerDialog::UpdateStatus()
{
	in->mName.Empty();
	in->mDescription.Empty();

	CWnd* btnEdit = GetDlgItem(IDC_BUTTON_EDIT);
	CWnd* btnDel = GetDlgItem(IDC_BUTTON_DELETE);
	ASSERT(btnEdit && btnDel);

	// 選択状態を見て選択中のコマンドを決定
	int itemIndex = 0;
	for (auto cmd : in->mCommands) {
		UINT mask = in->mListCtrl.GetItemState(itemIndex, LVIS_SELECTED | LVIS_FOCUSED);
		bool isSelItem = mask != 0;

		if (isSelItem) {
			in->mSelCommand = in->mCommands[itemIndex];
		}

		itemIndex++;
	}

	if (in->mSelCommand == nullptr) {
		btnEdit->EnableWindow(FALSE);
		btnDel->EnableWindow(FALSE);
		return false;
	}


	CString name = in->mSelCommand->GetName();

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	auto cmd = cmdRepoPtr->QueryAsWholeMatch(name);
	if (cmd) {
		in->mIconLabelPtr->DrawIcon(cmd->GetIcon());
		in->mName = name;
		in->mDescription = cmd->GetDescription();
		cmd->Release();
	}

	bool isBuiltin = cmdRepoPtr->IsBuiltinName(name);
	if (isBuiltin) {
		btnEdit->EnableWindow(FALSE);
		btnDel->EnableWindow(FALSE);
		return false;
	}

	btnEdit->EnableWindow(TRUE);
	btnDel->EnableWindow(TRUE);

	return true;
}

void KeywordManagerDialog::OnButtonNew()
{
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	cmdRepoPtr->NewCommandDialog();
	ResetContents();
}

void KeywordManagerDialog::OnButtonEdit()
{
	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	if (in->mSelCommand == nullptr) {
		return;
	}
	CString name = in->mSelCommand->GetName();
	if (cmdRepoPtr->IsBuiltinName(name)) {
		return;
	}

	cmdRepoPtr->EditCommandDialog(name);

	ResetContents();

	// 編集画面を閉じた後はキーワードマネージャー画面を操作できる状態にする
	SetForegroundWindow();
}

void KeywordManagerDialog::OnButtonDelete()
{
	if (in->mSelCommand == nullptr) {
		return ;
	}

	CString name = in->mSelCommand->GetName();

	CString confirmMsg((LPCTSTR)IDS_CONFIRM_DELETE);
	confirmMsg += _T("\n");
	confirmMsg += _T("\n");
	confirmMsg += name;

	int sel = AfxMessageBox(confirmMsg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	if (sel != IDYES) {
		return ;
	}

	auto cmdRepoPtr = soyokaze::core::CommandRepository::GetInstance();
	if (cmdRepoPtr->DeleteCommand(name) == false) {
		return ;
	}

	ResetContents();
	UpdateStatus();
	UpdateData(FALSE);
}

/**
 *  リスト欄の要素の状態変更時の処理
 */
void KeywordManagerDialog::OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	UpdateStatus();
	UpdateData(FALSE);
}

void KeywordManagerDialog::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;
	OnButtonEdit();
}

/**
 *  リスト欄のヘッダクリック時の処理
 */
void KeywordManagerDialog::OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult)
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
	int itemIndex = 0;
	for (auto cmd : in->mCommands) {
			bool isSelItem = cmd == in->mSelCommand;
			in->mListCtrl.SetItemState(itemIndex, isSelItem ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
			itemIndex++;
	}

	in->mListCtrl.Invalidate();
}

/**
 *  リスト欄のオーナーデータ周りの処理
 */
void KeywordManagerDialog::OnGetDispInfo(
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
				auto cmd = in->mCommands[itemIndex];
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetName(), _TRUNCATE);
			}
		}
		else if (pDispInfo->item.iSubItem == COL_CMDTYPE) {
			// 説明列のデータをコピー
			if (0 <= itemIndex && itemIndex < in->mCommands.size()) {
				auto cmd = in->mCommands[itemIndex];
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetTypeDisplayName(), _TRUNCATE);
			}
		}
		else if (pDispInfo->item.iSubItem == COL_DESCRIPTION) {
			// 説明列のデータをコピー
			if (0 <= itemIndex && itemIndex < in->mCommands.size()) {
				auto cmd = in->mCommands[itemIndex];
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetDescription(), _TRUNCATE);
			}
		}
	}
}

/**
 *  オーナーデータリストの検索処理
 */
void KeywordManagerDialog::OnFindCommand(
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
	if (startPos >= in->mCommands.size()) {
		startPos = 0;
	}

	int currentPos=startPos;

	// 検索開始位置からリスト末尾までを探す
	int commandCount = (int)in->mCommands.size();
	for (int i = startPos; i < commandCount; ++i) {

		// コマンド名を小文字に変換したうえで前方一致比較をする
		CString item = in->mCommands[i]->GetName();
		item.MakeLower();
		if (item.Find(searchStr) == 0) {
			*pResult = i;
			return;
		}
	}
	// 末尾まで行ってヒットしなかった場合は先頭から検索開始位置までを探す
	for (int i = 0; i < startPos; ++i) {

		CString item = in->mCommands[i]->GetName();
		item.MakeLower();

		if (item.Find(searchStr) == 0) {
			*pResult = i;
			return;
		}
	}
}
