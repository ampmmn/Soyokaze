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
	int mPriority{0};

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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

class CommandPriorityPageDialog : public CDialog
{
public:
	virtual ~CommandPriorityPageDialog();

	bool OnSetActive();
	bool OnKillActive();

	void UpdateListItems();
	bool UpdateStatus();
	void SortCommands();
	void SelectItem(Command* command, bool isRedraw);

	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;

// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnEditFilterChanged();
	afx_msg void OnButtonEdit();
	afx_msg void OnButtonResetAll();
	afx_msg void OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnFindCommand(NMHDR* pNMHDR, LRESULT* pResult);

	CommandRanking* mCommandPriority{nullptr};

	// コマンド一覧を表示するリストコントロール
	CListCtrl mListCtrl;
	// すべてのコマンド
	std::vector<Command*> mCommands;
	// 選択中のコマンド
	Command* mSelCommand{nullptr};
	// フィルタで絞り込みされた結果、画面に表示されているコマンド一覧
	std::vector<Command*> mShowCommands;

	// 現在のソート種別
	int mSortType{SORT_ASCEND_NAME};
	//! フィルター欄の文字列
	CString mFilterStr;
};

CommandPriorityPageDialog::~CommandPriorityPageDialog()
{
	for (auto cmd : mCommands) {
		cmd->Release();
	}

	if (mCommandPriority) {
		mCommandPriority->Release();
	}
}

/**
	コマンドをソートする
*/
void CommandPriorityPageDialog::SortCommands()
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
	@param[in] command 選択対象コマンド
	@param[in] isRedraw     再描画するか?
*/
void CommandPriorityPageDialog::SelectItem(
	Command* command,
	bool isRedraw
)
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

	if (isRedraw) {
		mListCtrl.Invalidate();
	}
}

bool CommandPriorityPageDialog::OnKillActive()
{
	if (UpdateData() == FALSE) {
		return false;
	}
	return true;
}

bool CommandPriorityPageDialog::OnSetActive()
{
	UpdateStatus();
	UpdateData(FALSE);
	return true;
}

void CommandPriorityPageDialog::OnOK()
{
	// 設定を書き戻す
	ASSERT(mCommandPriority);
	mCommandPriority->CopyTo(CommandRanking::GetInstance());

	// 優先度情報をファイルに保存する
	CommandRanking::GetInstance()->Save();

	__super::OnOK();
}

void CommandPriorityPageDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILTER, mFilterStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(CommandPriorityPageDialog, CDialog)
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

BOOL CommandPriorityPageDialog::OnInitDialog()
{
	__super::OnInitDialog();

	// 優先度情報を複製する
	// (OKで画面を閉じるときに確定するため、一時的な領域に情報を保持しておく)
	mCommandPriority = CommandRanking::GetInstance()->CloneTemporarily();

	mListCtrl.SubclassDlgItem(IDC_LIST_COMMANDS, this);

	// リスト　スタイル変更
	mListCtrl.SetExtendedStyle(mListCtrl.GetExtendedStyle()|LVS_EX_FULLROWSELECT);

	// ヘッダー追加
	LVCOLUMN lvc;
	memset(&lvc,0,sizeof(LV_COLUMN));
	lvc.mask = LVCF_TEXT|LVCF_FMT|LVCF_WIDTH;

	CString strHeader;
	strHeader.LoadString(IDS_NAME);
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	mListCtrl.InsertColumn(0,&lvc);

	strHeader = _T("優先度");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	mListCtrl.InsertColumn(1,&lvc);

	// コマンド一覧を取得する
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->EnumCommands(mCommands);

	// 現在のソート方法に従って要素をソート
	SortCommands();

	UpdateListItems();

	UpdateData(FALSE);

	return TRUE;
}

bool CommandPriorityPageDialog::UpdateStatus()
{
	POSITION pos = mListCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		int selItemIndex = mListCtrl.GetNextSelectedItem(pos);
		mSelCommand = mShowCommands[selItemIndex];
	}
	else {
		mSelCommand = nullptr;
	}

	// 選択項目がなければ編集ボタンを無効化する
	bool canEdit = mSelCommand != nullptr;
	GetDlgItem(IDC_BUTTON_EDIT)->EnableWindow(canEdit);

	return true;
}

void CommandPriorityPageDialog::UpdateListItems()
{
	// フィルタ欄が空でない場合は絞り込みを行う
	if (mFilterStr.IsEmpty() == FALSE) {
		RefPtr<Pattern> pattern(PartialMatchPattern::Create());
		pattern->SetWholeText(mFilterStr);

		mShowCommands.clear();
		for (auto& cmd : mCommands) {

			auto name = cmd->GetName();
			auto desc = cmd->GetDescription();
			auto typeName = cmd->GetTypeDisplayName();
			
			if (pattern->Match(name) == Pattern::Mismatch &&
			    pattern->Match(desc) == Pattern::Mismatch &&
			    pattern->Match(typeName) == Pattern::Mismatch)  {
				continue;
			}
			mShowCommands.push_back(cmd);
		}
	}
	else {
		mShowCommands = mCommands;
	}

	// アイテム数を設定
	int visibleItems = (int)(mShowCommands.size());
	mListCtrl.SetItemCountEx(visibleItems);

	// 選択状態の更新
	SelectItem(mSelCommand, true);
}

// フィルター欄の文字列が更新されたときの処理
void CommandPriorityPageDialog::OnEditFilterChanged()
{
	UpdateData();
	UpdateListItems();
	UpdateStatus();
}


// 編集ボタンが押下されたときの処理
void CommandPriorityPageDialog::OnButtonEdit()
{
	auto rank = mCommandPriority;

	PriorityDialog dlg(this);

	// 選択されている要素の優先度を得て、優先度入力画面に渡す
	int itemIndex = 0;
	for (auto& cmd : mShowCommands) {

		int state = mListCtrl.GetItemState(itemIndex++, LVIS_SELECTED);
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
	for (auto& cmd : mShowCommands) {

		// 選択されている要素を探し、優先度を更新する
		int state = mListCtrl.GetItemState(itemIndex++, LVIS_SELECTED);
		if (state == 0) {
			continue;
		}

		rank->Set(cmd, newPriority);
	}

	// 画面の再描画
	mListCtrl.Invalidate();
}

// すべてリセット ボタン押下時の処理
void CommandPriorityPageDialog::OnButtonResetAll()
{
	mCommandPriority->ResetAll();

	// 画面の再描画
	mListCtrl.Invalidate();
}

void CommandPriorityPageDialog::OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	UpdateStatus();
}

// リスト要素をダブルクリックしたときの処理
void CommandPriorityPageDialog::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	// 編集ボタン押下時の処理をそのまま実行する
	OnButtonEdit();
}


/**
 *  リスト欄のヘッダクリック時の処理
 */
void CommandPriorityPageDialog::OnHeaderClicked(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	NM_LISTVIEW* pNMLV = (NM_LISTVIEW*)pNMHDR;

	// クリックされた列で昇順/降順ソートをする

	int clickedCol = pNMLV->iSubItem;

	if(clickedCol == COL_CMDNAME) {
		// ソート方法の変更(コマンド名でソート)
		mSortType = mSortType == SORT_ASCEND_NAME ? SORT_DESCEND_NAME : SORT_ASCEND_NAME;
	}
	else if (clickedCol == COL_PRIORITY) {
		// ソート方法の変更(説明でソート)
		mSortType = mSortType == SORT_ASCEND_PRIORITY ? SORT_DESCEND_PRIORITY : SORT_ASCEND_PRIORITY;
	}
	// ソート実施
	SortCommands();

	// 選択状態の更新
	UpdateListItems();
}

/**
 *  リスト欄のオーナーデータ周りの処理
 */
void CommandPriorityPageDialog::OnGetDispInfo(
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
			if (0 <= itemIndex && itemIndex < mShowCommands.size()) {
				auto cmd = mShowCommands[itemIndex];
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, cmd->GetName(), _TRUNCATE);
			}
		}
		else if (pDispInfo->item.iSubItem == COL_PRIORITY) {
			// 優先度列のデータをコピー

			const CommandRanking* rankPtr = mCommandPriority;

			if (0 <= itemIndex && itemIndex < mShowCommands.size()) {
				auto cmd = mShowCommands[itemIndex];
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
void CommandPriorityPageDialog::OnFindCommand(
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
	if (startPos >= mShowCommands.size()) {
		startPos = 0;
	}

	// 検索開始位置からリスト末尾までを探す
	int commandCount = (int)mShowCommands.size();
	for (int i = startPos; i < commandCount; ++i) {

		// コマンド名を小文字に変換したうえで前方一致比較をする
		CString item = mShowCommands[i]->GetName();
		item.MakeLower();
		if (item.Find(searchStr) == 0) {
			*pResult = i;
			return;
		}
	}
	// 末尾まで行ってヒットしなかった場合は先頭から検索開始位置までを探す
	for (int i = 0; i < startPos; ++i) {

		CString item = mCommands[i]->GetName();
		item.MakeLower();

		if (item.Find(searchStr) == 0) {
			*pResult = i;
			return;
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct AppSettingPageCommandPriority::PImpl
{
	CommandPriorityPageDialog mWindow;
};

REGISTER_APPSETTINGPAGE(AppSettingPageCommandPriority)

AppSettingPageCommandPriority::AppSettingPageCommandPriority() : 
	AppSettingPageBase(_T("実行"), _T("優先度")),
	in(new PImpl)
{
}

AppSettingPageCommandPriority::~AppSettingPageCommandPriority()
{
}

// ウインドウを作成する
bool AppSettingPageCommandPriority::Create(HWND parentWindow)
{
	return in->mWindow.Create(IDD_APPSETTING_COMMANDPRIORITY, CWnd::FromHandle(parentWindow)) != FALSE;
}

// ウインドウハンドルを取得する
HWND AppSettingPageCommandPriority::GetHwnd()
{
	return in->mWindow.GetSafeHwnd();
}

// 同じ親の中で表示する順序(低いほど先に表示)
int AppSettingPageCommandPriority::GetOrder()
{
	return 50;
}
// 
bool AppSettingPageCommandPriority::OnEnterSettings()
{
	return true;
}

// ページがアクティブになるときに呼ばれる
bool AppSettingPageCommandPriority::OnSetActive()
{
	return in->mWindow.OnSetActive();
}

// ページが非アクティブになるときに呼ばれる
bool AppSettingPageCommandPriority::OnKillActive()
{
	return in->mWindow.OnKillActive();
}
//
void AppSettingPageCommandPriority::OnOKCall()
{
	in->mWindow.OnOK();
}

// ページに関連付けられたヘルプページIDを取得する
bool AppSettingPageCommandPriority::GetHelpPageId(String& id)
{
	id = "CommandPrioritySetting";
	return true;
}

