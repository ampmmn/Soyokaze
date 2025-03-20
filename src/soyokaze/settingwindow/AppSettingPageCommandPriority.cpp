#include "pch.h"
#include "framework.h"
#include "AppSettingPageCommandPriority.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRanking.h"
#include "matcher/PartialMatchPattern.h"
#include "setting/Settings.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace core {

/**
 *  コマンド優先度設定ダイアログ
 */
class PriorityDialog : public CDialogEx
{
public:
	PriorityDialog(CWnd* parent) : CDialogEx(IDD_PRIORITY, parent)
	{
	}

	virtual ~PriorityDialog() {}

	//! 優先度
	int mPriority = 0;

	void DoDataExchange(CDataExchange* pDX) override {
		__super::DoDataExchange(pDX);
		DDX_Text(pDX, IDC_EDIT_PRIORITY, mPriority);
		DDV_MinMaxInt(pDX, mPriority, 0, 10000);
	}

	void OnOK() override
	{
		if (UpdateData() == FALSE) {
			return;
		}
		__super::OnOK();
	}

// 実装
protected:
	DECLARE_MESSAGE_MAP()
};

BEGIN_MESSAGE_MAP(PriorityDialog, CDialogEx)
END_MESSAGE_MAP()

}
}
}



using Command = launcherapp::core::Command;
using CommandRanking = launcherapp::commands::core::CommandRanking;
using PriorityDialog = launcherapp::commands::core::PriorityDialog;

// リストの列情報
enum {
	COL_CMDNAME,      // コマンド名
	COL_PRIORITY,     // 優先度
};

// ソート状態
enum {
	SORT_ASCEND_NAME,       // コマンド名-昇順
	SORT_DESCEND_NAME,      // コマンド名-降順
	SORT_ASCEND_PRIORITY,   // 説明-昇順
	SORT_DESCEND_PRIORITY,  // 説明-降順
};


struct AppSettingPageCommandPriority::PImpl
{
	void SortCommands();
	void SelectItem(int selItemInedex, bool isRedraw);

	CommandRanking* mCommandPriority = nullptr;

	// コマンド一覧を表示するリストコントロール
	CListCtrl mListCtrl;
	// すべてのコマンド
	std::vector<Command*> mCommands;
	// フィルタで絞り込みされた結果、画面に表示されているコマンド一覧
	std::vector<Command*> mShowCommands;

	// 現在のソート種別
	int mSortType = SORT_ASCEND_NAME;
	//! フィルター欄の文字列
	CString mFilterStr;

};

/**
	コマンドをソートする
*/
void AppSettingPageCommandPriority::PImpl::SortCommands()
{
	if (mSortType == SORT_ASCEND_NAME) {
		// 名前昇順
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return l->GetName().CompareNoCase(r->GetName()) < 0;
		});
		return;
	}
	else if (mSortType == SORT_DESCEND_NAME) {
		// 名前降順
		std::sort(mCommands.begin(), mCommands.end(), [](Command* l, Command* r) {
			return r->GetName().CompareNoCase(l->GetName()) < 0;
		});
		return;
	}

	const CommandRanking* rankPtr = mCommandPriority;
	if (mSortType == SORT_ASCEND_PRIORITY) {
		std::sort(mCommands.begin(), mCommands.end(), [rankPtr](Command* l, Command* r) {
		// 優先度昇順
			int priorityL = rankPtr->Get(l);
			int priorityR = rankPtr->Get(r);
			return priorityR < priorityL;
		});
	}
	else if (mSortType == SORT_DESCEND_PRIORITY) {
		// 優先度降順
		std::sort(mCommands.begin(), mCommands.end(), [rankPtr](Command* l, Command* r) {
			int priorityL = rankPtr->Get(l);
			int priorityR = rankPtr->Get(r);
			return priorityL < priorityR;
		});
	}
}

/**
  選択状態の更新
	@param[in] selItemIndex リスト要素のインデックス
	@param[in] isRedraw     再描画するか?
*/
void AppSettingPageCommandPriority::PImpl::SelectItem(
	int selItemIndex,
	bool isRedraw
)
{
	int itemIndex = 0;
	for (auto& cmd : mShowCommands) {

		bool isSelItem = itemIndex== selItemIndex;
		mListCtrl.SetItemState(itemIndex, isSelItem ? LVIS_SELECTED | LVIS_FOCUSED : 0, LVIS_SELECTED | LVIS_FOCUSED);
		itemIndex++;
	}
	mListCtrl.EnsureVisible(selItemIndex, FALSE);

	if (isRedraw) {
		mListCtrl.Invalidate();
	}
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



AppSettingPageCommandPriority::AppSettingPageCommandPriority(CWnd* parentWnd) : 
	SettingPage(_T("優先度"), IDD_APPSETTING_COMMANDPRIORITY, parentWnd),
	in(std::make_unique<PImpl>())
{
}

AppSettingPageCommandPriority::~AppSettingPageCommandPriority()
{
	if (in->mCommandPriority) {
		in->mCommandPriority->Release();
	}
}

BOOL AppSettingPageCommandPriority::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return FALSE;
	}
	return TRUE;
}

BOOL AppSettingPageCommandPriority::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return TRUE;
}

void AppSettingPageCommandPriority::OnOK()
{
	// 設定を書き戻す
	ASSERT(in->mCommandPriority);
	in->mCommandPriority->CopyTo(CommandRanking::GetInstance());

	// 優先度情報をファイルに保存する
	CommandRanking::GetInstance()->Save();

	__super::OnOK();
}

void AppSettingPageCommandPriority::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILTER, in->mFilterStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(AppSettingPageCommandPriority, SettingPage)
	ON_EN_CHANGE(IDC_EDIT_FILTER, OnEditFilterChanged)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_RESET, OnButtonResetAll)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMANDS, OnLvnItemChange)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COMMANDS, OnNMDblclk)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_COMMANDS, OnHeaderClicked)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_COMMANDS, OnGetDispInfo)
	ON_NOTIFY(LVN_ODFINDITEM , IDC_LIST_COMMANDS, OnFindCommand)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL AppSettingPageCommandPriority::OnInitDialog()
{
	__super::OnInitDialog();

	// 優先度情報を複製する
	// (OKで画面を閉じるときに確定するため、一時的な領域に情報を保持しておく)
	in->mCommandPriority = CommandRanking::GetInstance()->CloneTemporarily();

	in->mListCtrl.SubclassDlgItem(IDC_LIST_COMMANDS, this);

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

	strHeader = _T("優先度");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	in->mListCtrl.InsertColumn(1,&lvc);

	// コマンド一覧を取得する
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->EnumCommands(in->mCommands);

	// 現在のソート方法に従って要素をソート
	in->SortCommands();

	UpdateListItems();

	UpdateData(FALSE);

	return TRUE;
}

bool AppSettingPageCommandPriority::UpdateStatus()
{
	// 更新前の選択位置を覚えておく
	int selItemIndex = -1;
	POSITION pos = in->mListCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		selItemIndex = in->mListCtrl.GetNextSelectedItem(pos);
	}

	// 選択項目がなければ編集ボタンを無効化する
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(selItemIndex != -1);

	return true;
}


void AppSettingPageCommandPriority::OnEnterSettings()
{
	// auto settingsPtr = (Settings*)GetParam();
	// in->mIsWarnLongOperation = (BOOL)settingsPtr->Get(_T("Health:IsWarnLongOperation"), false);
}

bool AppSettingPageCommandPriority::GetHelpPageId(CString& id)
{
	id = _T("CommandPrioritySetting");
	return true;
}


void AppSettingPageCommandPriority::UpdateListItems()
{
	// 更新前の選択位置を覚えておく
	int selItemIndex = -1;
	POSITION pos = in->mListCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		selItemIndex = in->mListCtrl.GetNextSelectedItem(pos);
	}

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

	// 選択項目がなければ編集ボタンを無効化する
	bool canEdit = selItemIndex != -1 && selItemIndex < in->mShowCommands.size();
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(canEdit);

	// アイテム数を設定
	int visibleItems = (int)(in->mShowCommands.size());
	in->mListCtrl.SetItemCountEx(visibleItems);

	// 選択状態の更新
	in->SelectItem(selItemIndex, true);
}

// フィルター欄の文字列が更新されたときの処理
void AppSettingPageCommandPriority::OnEditFilterChanged()
{
	UpdateData();
	UpdateListItems();
	UpdateStatus();
}


// 編集ボタンが押下されたときの処理
void AppSettingPageCommandPriority::OnButtonEdit()
{
	auto rank = in->mCommandPriority;

	PriorityDialog dlg(this);

	// 選択されている要素の優先度を得て、優先度入力画面に渡す
	int itemIndex = 0;
	for (auto& cmd : in->mShowCommands) {

		int state = in->mListCtrl.GetItemState(itemIndex++, LVIS_SELECTED);
		if (state == 0) {
			continue;
		}

		dlg.mPriority = rank->Get(cmd);
		break;
	}

	// 優先度入力画面を表示する
	if (dlg.DoModal() != IDOK) {
		return;
	}

	// 優先度を更新する
	int newPriority = dlg.mPriority;

	itemIndex = 0;
	for (auto& cmd : in->mShowCommands) {

		// 選択されている要素を探し、優先度を更新する
		int state = in->mListCtrl.GetItemState(itemIndex++, LVIS_SELECTED);
		if (state == 0) {
			continue;
		}

		rank->Set(cmd, newPriority);
	}

	// 画面の再描画
	in->mListCtrl.Invalidate();
}

// すべてリセット ボタン押下時の処理
void AppSettingPageCommandPriority::OnButtonResetAll()
{
	in->mCommandPriority->ResetAll();

	// 画面の再描画
	in->mListCtrl.Invalidate();
}

void AppSettingPageCommandPriority::OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	UpdateStatus();
}

// リスト要素をダブルクリックしたときの処理
void AppSettingPageCommandPriority::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	// 編集ボタン押下時の処理をそのまま実行する
	OnButtonEdit();
}


/**
 *  リスト欄のヘッダクリック時の処理
 */
void AppSettingPageCommandPriority::OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;

	// クリックされた列で昇順/降順ソートをする

	int clickedCol = pNMLV->iSubItem;

	if(clickedCol == COL_CMDNAME) {
		// ソート方法の変更(コマンド名でソート)
		in->mSortType = in->mSortType == SORT_ASCEND_NAME ? SORT_DESCEND_NAME : SORT_ASCEND_NAME;
	}
	else if (clickedCol == COL_PRIORITY) {
		// ソート方法の変更(説明でソート)
		in->mSortType = in->mSortType == SORT_ASCEND_PRIORITY ? SORT_DESCEND_PRIORITY : SORT_ASCEND_PRIORITY;
	}
	// ソート実施
	in->SortCommands();

	// 選択状態の更新
	UpdateListItems();
}

/**
 *  リスト欄のオーナーデータ周りの処理
 */
void AppSettingPageCommandPriority::OnGetDispInfo(
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
			if (0 <= itemIndex && itemIndex < in->mShowCommands.size()) {
				auto cmd = in->mShowCommands[itemIndex];
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetName(), _TRUNCATE);
			}
		}
		else if (pDispInfo->item.iSubItem == COL_PRIORITY) {
			// 優先度列のデータをコピー

			const CommandRanking* rankPtr = in->mCommandPriority;

			if (0 <= itemIndex && itemIndex < in->mShowCommands.size()) {
				auto cmd = in->mShowCommands[itemIndex];
				int priority = rankPtr->Get(cmd);
				TCHAR priorityStr[32] = {};
				_stprintf_s(priorityStr, _T("%d"),priority);
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, priorityStr, _TRUNCATE);
			}
		}
	}
}

/**
 *  オーナーデータリストの検索処理
 */
void AppSettingPageCommandPriority::OnFindCommand(
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

		CString item = in->mCommands[i]->GetName();
		item.MakeLower();

		if (item.Find(searchStr) == 0) {
			*pResult = i;
			return;
		}
	}
}
