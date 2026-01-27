#include "pch.h"
#include "framework.h"
#include "control/KeywordManagerDialog.h"
#include "control/KeywordEdit.h"
#include "icon/IconLabel.h"
#include "core/IFIDDefine.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/UserCommandProvider.h"
#include "commands/core/EditableIF.h"
#include "commands/core/CommandProviderRepository.h"
#include "commands/transfer/CommandClipboardTransfer.h"
#include "matcher/PartialMatchPattern.h"
#include "utility/RefPtr.h"
#include "hotkey/CommandHotKeyManager.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::core;

// リストの列情報
enum {
	COL_CMDNAME,      // コマンド名
	COL_CMDTYPE,      // 種別
	COL_DESCRIPTION,  // 説明
	COL_HOTKEY,       // ホットキー
};

// ソート状態
enum {
	SORT_ASCEND_NAME,          // コマンド名-昇順
	SORT_DESCEND_NAME,         // コマンド名-降順
	SORT_ASCEND_DESCRIPTION,   // 説明-昇順
	SORT_DESCEND_DESCRIPTION,  // 説明-降順
	SORT_ASCEND_CMDTYPE,   // 種別-昇順
	SORT_DESCEND_CMDTYPE,  // 種別-降順
	SORT_ASCEND_HOTKEY,   // ホットキー-昇順
	SORT_DESCEND_HOTKEY,  // ホットキー-降順
};


struct KeywordManagerDialog::PImpl : public CommandRepositoryListenerIF
{
	void SortCommands();
	void SelectItem(Command* command, bool isRedrawRequired);

	Command* GetItem(int index) {
		ASSERT(0 <= index && index < mShowCommands.size()); 
		return mShowCommands[index];
	}

	bool IsEditable();
	bool IsDeletable();

// CommandRepositoryListenerIF
	void OnBeforeLoad() override {}
	void OnNewCommand(Command*) override {}
	void OnDeleteCommand(Command*) override {}
	void OnPatternReloaded()
	{
		// ウインドウメッセージ経由で設定をリロードする
		::PostMessage(mHwnd, WM_APP+2, 0, 0);
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

	CommandHotKeyMappings mKeyMapping;

	int mSortType{SORT_ASCEND_NAME};
	HWND mHwnd{nullptr};
	HACCEL mAccel{nullptr};
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
	else if (mSortType == SORT_ASCEND_HOTKEY) {
		std::sort(mCommands.begin(), mCommands.end(), [&](Command* l, Command* r) {
		auto strL = mKeyMapping.FindKeyMappingString(l->GetName());
		auto strR = mKeyMapping.FindKeyMappingString(r->GetName());
			return strL < strR;
		});
	}
	else if (mSortType == SORT_DESCEND_HOTKEY) {
		std::sort(mCommands.begin(), mCommands.end(), [&](Command* l, Command* r) {
		auto strL = mKeyMapping.FindKeyMappingString(l->GetName());
		auto strR = mKeyMapping.FindKeyMappingString(r->GetName());
			return strR < strL;
		});
	}
}

// 選択状態の更新
void KeywordManagerDialog::PImpl::SelectItem(Command* command, bool isRedrawRequired)
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

bool KeywordManagerDialog::PImpl::IsEditable()
{
	if (mSelCommand == nullptr) {
		return false;
	}

	RefPtr<Editable> editable;
	if (mSelCommand->QueryInterface(IFID_EDITABLE, (void**)&editable) == false) {
		return false;
	}
	if (editable->IsEditable() == false) {
		return false;
	}
	return true;
}

bool KeywordManagerDialog::PImpl::IsDeletable()
{
	if (mSelCommand == nullptr) {
		return false;
	}

	RefPtr<Editable> editable;
	if (mSelCommand->QueryInterface(IFID_EDITABLE, (void**)&editable) == false) {
		return false;
	}
	if (editable->IsDeletable() == false) {
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

KeywordManagerDialog::KeywordManagerDialog() : 
	launcherapp::control::SinglePageDialog(IDD_KEYWORDMANAGER),
	in(std::make_unique<PImpl>())
{
	SetHelpPageId("KeywordManager");

	in->mIconLabelPtr = std::make_unique<IconLabel>();
	in->mSortType = SORT_ASCEND_NAME;

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->RegisterListener(in.get());

	ACCEL accels[2] = {
		{ FCONTROL | FVIRTKEY, 'C', ID_EDIT_COPY },
		{ FCONTROL | FVIRTKEY, 'V', ID_EDIT_PASTE },
	};
	in->mAccel = CreateAcceleratorTable(accels, 2);
}

KeywordManagerDialog::~KeywordManagerDialog()
{
	if (in->mAccel) {
		DestroyAcceleratorTable(in->mAccel);
	}
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->UnregisterListener(in.get());

	for (auto& cmd : in->mCommands) {
		cmd->Release();
	}
}

void KeywordManagerDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_NAME, in->mName);
	DDX_Text(pDX, IDC_STATIC_DESCRIPTION, in->mDescription);
	DDX_Text(pDX, IDC_EDIT_FILTER, in->mFilterStr);
}

#pragma warning( push )
#pragma warning( disable : 26454 )

BEGIN_MESSAGE_MAP(KeywordManagerDialog, launcherapp::control::SinglePageDialog)
	ON_EN_CHANGE(IDC_EDIT_FILTER, OnEditFilterChanged)
	ON_COMMAND(IDC_BUTTON_NEW, OnButtonNew)
	ON_COMMAND(IDC_BUTTON_EDIT, OnButtonEdit)
	ON_COMMAND(IDC_BUTTON_CLONE, OnButtonClone)
	ON_COMMAND(IDC_BUTTON_DELETE, OnButtonDelete)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST_COMMANDS, OnLvnItemChange)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_COMMANDS, OnNMDblclk)
	ON_NOTIFY(LVN_COLUMNCLICK, IDC_LIST_COMMANDS, OnHeaderClicked)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_COMMANDS, OnGetDispInfo)
	ON_NOTIFY(LVN_ODFINDITEM , IDC_LIST_COMMANDS, OnFindCommand)
	ON_MESSAGE(WM_APP+1, OnUserMessageKeywrodEditKeyDown)
	ON_MESSAGE(WM_APP+2, OnUserMessageResetContent)
END_MESSAGE_MAP()

#pragma warning( pop )

BOOL KeywordManagerDialog::OnInitDialog()
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

	strHeader = _T("ホットキー");
	lvc.pszText = const_cast<LPTSTR>((LPCTSTR)strHeader);
	lvc.cx = 100;
	lvc.fmt = LVCFMT_LEFT;
	in->mListCtrl.InsertColumn(3,&lvc);

	ResetContents();

	UpdateStatus();
	UpdateData(FALSE);

	return TRUE;
}

BOOL KeywordManagerDialog::PreTranslateMessage(MSG* pMsg)
{
	if (in->mAccel && TranslateAccelerator(GetSafeHwnd(), in->mAccel, pMsg)) {
		return TRUE;
	}
	return __super::PreTranslateMessage(pMsg);
}

void KeywordManagerDialog::ResetContents()
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

	// ホットキー一覧を取得
	CommandHotKeyManager::GetInstance()->GetMappings(in->mKeyMapping);
	
	// 現在のソート方法に従って要素をソート
	in->SortCommands();

	UpdateListItems();
}

bool KeywordManagerDialog::UpdateStatus()
{
	POSITION pos = in->mListCtrl.GetFirstSelectedItemPosition();
	if (pos) {
		int selItemIndex = in->mListCtrl.GetNextSelectedItem(pos);
		in->mSelCommand = in->mShowCommands[selItemIndex];
	}
	else {
		in->mSelCommand = nullptr;
	}

	in->mName.Empty();
	in->mDescription.Empty();

	CWnd* btnEdit = GetDlgItem(IDC_BUTTON_EDIT);
	CWnd* btnClone = GetDlgItem(IDC_BUTTON_CLONE);
	CWnd* btnDel = GetDlgItem(IDC_BUTTON_DELETE);
	ASSERT(btnEdit && btnClone && btnDel);

	if (in->mSelCommand == nullptr) {
		btnEdit->EnableWindow(FALSE);
		btnClone->EnableWindow(FALSE);
		btnDel->EnableWindow(FALSE);
		return false;
	}

	CString name = in->mSelCommand->GetName();

	in->mIconLabelPtr->DrawIcon(in->mSelCommand->GetIcon());
	in->mName = name;
	in->mDescription = in->mSelCommand->GetDescription();

	bool isEditable = in->IsEditable();
	btnEdit->EnableWindow(isEditable ? TRUE : FALSE);

	bool isDeletable = in->IsDeletable();
	btnClone->EnableWindow(isDeletable ? TRUE : FALSE);
	btnDel->EnableWindow(isDeletable ? TRUE : FALSE);

	return true;
}

void KeywordManagerDialog::UpdateListItems()
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

void KeywordManagerDialog::OnEditFilterChanged()
{
	UpdateData();
	UpdateListItems();
	UpdateStatus();
}

void KeywordManagerDialog::OnButtonNew()
{
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->NewCommandDialog();
	ResetContents();
}

void KeywordManagerDialog::OnButtonEdit()
{
	// 編集不可ならしない
	if (in->IsEditable() == false) {
		return;
	}

	CString name = in->mSelCommand->GetName();

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->EditCommandDialog(name, false);

	// 編集画面を閉じた後はキーワードマネージャー画面を操作できる状態にする
	SetForegroundWindow();
}

void KeywordManagerDialog::OnButtonClone()
{
	// 削除不可ならしない(削除できないコマンドは複製させない)
	if (in->IsDeletable() == false) {
		return;
	}

	CString name = in->mSelCommand->GetName();

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->EditCommandDialog(name, true);

	// 編集画面を閉じた後はキーワードマネージャー画面を操作できる状態にする
	SetForegroundWindow();
}

void KeywordManagerDialog::OnButtonDelete()
{
	// 削除不可なら許可しない
	if (in->IsDeletable() == false) {
		return;
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

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->UnregisterCommand(in->mSelCommand);

	ResetContents();
	UpdateStatus();
	UpdateData(FALSE);
}

void KeywordManagerDialog::OnEditCopy()
{
	auto cmd = in->mSelCommand;
	if (cmd == nullptr) {
		// 何も選択されていなければ何もしない
		return;
	}

	auto transfer = launcherapp::commands::transfer::CommandClipboardTransfer::GetInstance();
	auto entry = transfer->NewEntry(cmd->GetName());
	if (cmd->Save(entry) == false) {
		spdlog::error(_T("Failed to save command.{}"), (LPCTSTR)cmd->GetName());
		entry->Release();
		return ;
	}
	transfer->SendEntry(entry);
}

void KeywordManagerDialog::OnEditPaste()
{
	auto transfer = launcherapp::commands::transfer::CommandClipboardTransfer::GetInstance();

	RefPtr<CommandEntryIF> entry;
	if (transfer->ReceiveEntry(&entry) == false) {
		return;
	}

	auto providerRepos = launcherapp::core::CommandProviderRepository::GetInstance();

	std::vector<launcherapp::core::CommandProvider*> providers;
	providerRepos->EnumProviders(providers);

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	for (auto provider : providers) {
		RefPtr<launcherapp::core::UserCommandProvider> userCmdProvider;
		if (provider->QueryInterface(IFID_USERCOMMANDPROVIDER, (void**)&userCmdProvider) == false) {
			continue;
		}
		RefPtr<Command> newCmd;
		if (userCmdProvider->LoadFrom(entry.get(), &newCmd) == false) {
			continue;
		}

		RefPtr<Command> orgCmd(cmdRepoPtr->QueryAsWholeMatch(newCmd->GetName()));
		if (orgCmd.get() == nullptr) {
			newCmd->AddRef();
			cmdRepoPtr->RegisterCommand(newCmd.get());
		}
		else {
			// 上書きするか確認
			CString overwriteConfirmMsg;
			overwriteConfirmMsg.Format(_T("コマンド %s は既に存在します。\n設定を上書きしてよろしいですか?"),
			                           (LPCTSTR)newCmd->GetName());
			if (AfxMessageBox(overwriteConfirmMsg, MB_OKCANCEL | MB_ICONQUESTION) != IDOK) {
				return;
			}

			// 上書きする(ので前のコマンドを削除)
			cmdRepoPtr->UnregisterCommand(orgCmd.get());

			// 新しい方のコマンドを登録
			newCmd->AddRef();
			cmdRepoPtr->RegisterCommand(newCmd.get());
		}

		// リスト再描画
		ResetContents();

		// インポートしたコマンドを選択状態にする
		in->mSelCommand = newCmd.get();
		in->SelectItem(newCmd.get(), false);
		break;
	}
}

/**
 *  リスト欄の要素の状態変更時の処理
 */
void KeywordManagerDialog::OnLvnItemChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
	UpdateStatus();
	UpdateData(FALSE);
}

void KeywordManagerDialog::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	UNREFERENCED_PARAMETER(pNMHDR);

	*pResult = 0;
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
	else if (clickedCol == COL_HOTKEY) {
		// ソート方法の変更(ホットキーでソート)
		in->mSortType = in->mSortType == SORT_ASCEND_HOTKEY ? SORT_DESCEND_HOTKEY : SORT_ASCEND_HOTKEY;
	}

	// ソート実施
	in->SortCommands();

	// 選択状態の更新
	UpdateListItems();
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
		else if (pDispInfo->item.iSubItem == COL_HOTKEY) {
			// ホットキーの文字列
			if (0 <= itemIndex && itemIndex < in->mCommands.size()) {
				auto cmd = in->GetItem(itemIndex);
				auto mappingStr = in->mKeyMapping.FindKeyMappingString(cmd->GetName());
				_tcsncpy_s(pItem->pszText, pItem->cchTextMax, mappingStr, _TRUNCATE);
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

LRESULT KeywordManagerDialog::OnUserMessageKeywrodEditKeyDown(WPARAM wParam, LPARAM lParam)
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

LRESULT KeywordManagerDialog::OnUserMessageResetContent(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(wParam);
	UNREFERENCED_PARAMETER(lParam);

	ResetContents();
	spdlog::info("KeywordManager content updated.");

	return 0;
}

