#include "pch.h"
#include "framework.h"
#include "features/keywordmanager/KeywordManagerDialog.h"
#include "control/KeywordEdit.h"
#include "icon/IconLabel.h"
#include "core/IFIDDefine.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandFile.h"
#include "commands/core/CommandFileEntry.h"
#include "features/keywordmanager/CommandImportNameResolver.h"
#include "commands/core/UserCommandProvider.h"
#include "commands/core/EditableIF.h"
#include "commands/core/CommandProviderRepository.h"
#include "commands/transfer/CommandClipboardTransfer.h"
#include "features/keywordmanager/ImportCommandsDialog.h"
#include "matcher/PartialMatchPattern.h"
#include "utility/RefPtr.h"
#include "hotkey/CommandHotKeyManager.h"
#include "icon/IconLoader.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr UINT TIMERID_FILTER = 1;

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
	void SelectItems(const std::vector<Command*>& commands, bool isRedrawRequired);
	std::vector<Command*> GetSelectedCommands();

	Command* GetItem(int index) {
		ASSERT(0 <= index && index < mShowCommands.size()); 
		return mShowCommands[index];
	}

	bool IsEditable();
	bool IsDeletable();
	bool IsDeletable(Command* command);
	bool IsUserCommand(Command* command);

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

	// フィルター更新タイマーID
	UINT_PTR mUpdateTimerId{0};
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
	std::vector<Command*> commands;
	if (command != nullptr) {
		commands.push_back(command);
	}
	SelectItems(commands, isRedrawRequired);
}

void KeywordManagerDialog::PImpl::SelectItems(const std::vector<Command*>& commands, bool isRedrawRequired)
{
	int firstSelectedIndex = -1;

	int itemIndex = 0;
	for (auto& cmd : mShowCommands) {
		bool isSelItem = std::find(commands.begin(), commands.end(), cmd) != commands.end();
		bool isFocusedItem = isSelItem && firstSelectedIndex == -1;
		if (isSelItem) {
			if (firstSelectedIndex == -1) {
				firstSelectedIndex = itemIndex;
			}
		}
		UINT state = isSelItem ? LVIS_SELECTED : 0;
		if (isFocusedItem) {
			state |= LVIS_FOCUSED;
		}
		mListCtrl.SetItemState(itemIndex, state, LVIS_SELECTED | LVIS_FOCUSED);
		itemIndex++;
	}
	if (firstSelectedIndex != -1) {
		mListCtrl.EnsureVisible(firstSelectedIndex, FALSE);
	}

	if (isRedrawRequired) {
		mListCtrl.Invalidate();
	}
}

std::vector<Command*> KeywordManagerDialog::PImpl::GetSelectedCommands()
{
	std::vector<Command*> commands;
	POSITION pos = mListCtrl.GetFirstSelectedItemPosition();
	while (pos != nullptr) {
		int index = mListCtrl.GetNextSelectedItem(pos);
		if (0 <= index && index < (int)mShowCommands.size()) {
			commands.push_back(mShowCommands[index]);
		}
	}
	return commands;
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
	return IsDeletable(mSelCommand);
}


bool KeywordManagerDialog::PImpl::IsDeletable(Command* command)
{
	if (command == nullptr) {
		return false;
	}
	RefPtr<Editable> editable;
	if (command->QueryInterface(IFID_EDITABLE, (void**)&editable) == false) {
		return false;
	}
	if (editable->IsDeletable() == false) {
		return false;
	}
	return true;
}

bool KeywordManagerDialog::PImpl::IsUserCommand(Command* command)
{
	// ユーザーコマンドは削除可能、組み込みコマンドは削除不可として扱う
	return IsDeletable(command);
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
	ON_COMMAND(IDC_BUTTON_IMPORT, OnButtonImport)
	ON_COMMAND(IDC_BUTTON_EXPORT, OnButtonExport)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
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

BOOL KeywordManagerDialog::OnInitDialog()
{
	__super::OnInitDialog();

	in->mHwnd = GetSafeHwnd();

	SetIcon(IconLoader::Get()->LoadKeywordManagerIcon(), FALSE);

	in->mListCtrl.SubclassDlgItem(IDC_LIST_COMMANDS, this);
	in->mListCtrl.ModifyStyle(LVS_SINGLESEL, 0);
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

void KeywordManagerDialog::OnCancel()
{
	if (in->mUpdateTimerId != 0) {
		KillTimer(TIMERID_FILTER);
	}
	__super::OnCancel();
}

void KeywordManagerDialog::ResetContents()
{
	auto selectedCommands = in->GetSelectedCommands();
	in->SelectItems({}, false);

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
	in->SelectItems(selectedCommands, true);
}

bool KeywordManagerDialog::UpdateStatus()
{
	auto selectedCommands = in->GetSelectedCommands();
	in->mSelCommand = selectedCommands.empty() ? nullptr : selectedCommands.front();

	in->mName.Empty();
	in->mDescription.Empty();

	CWnd* btnNew = GetDlgItem(IDC_BUTTON_NEW);
	CWnd* btnEdit = GetDlgItem(IDC_BUTTON_EDIT);
	CWnd* btnClone = GetDlgItem(IDC_BUTTON_CLONE);
	CWnd* btnDel = GetDlgItem(IDC_BUTTON_DELETE);
	CWnd* btnExport = GetDlgItem(IDC_BUTTON_EXPORT);
	ASSERT(btnNew && btnEdit && btnClone && btnDel && btnExport);
	btnNew->EnableWindow(TRUE);

	if (selectedCommands.empty()) {
		btnEdit->EnableWindow(FALSE);
		btnClone->EnableWindow(FALSE);
		btnDel->EnableWindow(FALSE);
		btnExport->EnableWindow(FALSE);
		return false;
	}

	CString name = in->mSelCommand->GetName();

	in->mIconLabelPtr->DrawIcon(in->mSelCommand->GetIcon());
	in->mName = name;
	in->mDescription = in->mSelCommand->GetDescription();

	bool isSingleSelection = selectedCommands.size() == 1;
	btnEdit->EnableWindow(isSingleSelection && in->IsEditable() ? TRUE : FALSE);
	btnClone->EnableWindow(isSingleSelection && in->IsDeletable() ? TRUE : FALSE);

	bool hasDeletableCommand = std::any_of(selectedCommands.begin(), selectedCommands.end(), [&](Command* command) {
		return in->IsDeletable(command);
	});
	btnDel->EnableWindow(hasDeletableCommand ? TRUE : FALSE);

	bool hasUserCommand = std::any_of(selectedCommands.begin(), selectedCommands.end(), [&](Command* command) {
		return in->IsUserCommand(command);
	});
	btnExport->EnableWindow(hasUserCommand ? TRUE : FALSE);

	return true;
}

void KeywordManagerDialog::UpdateListItems()
{
	auto selectedCommands = in->GetSelectedCommands();

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

	// 並び替えや絞り込み後も、表示対象に残っている選択を復元する
	in->SelectItems(selectedCommands, true);
}

void KeywordManagerDialog::OnEditFilterChanged()
{
	// フィルター欄更新のつど更新するのではなく、
	// 最後のキー入力後の0.2秒後に更新を入れる

	if (in->mUpdateTimerId != 0) {
		KillTimer(TIMERID_FILTER);
	}
	in->mUpdateTimerId = SetTimer(TIMERID_FILTER, 200, 0);
}

void KeywordManagerDialog::OnTimer(UINT_PTR timerId)
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

void KeywordManagerDialog::OnButtonNew()
{
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->NewCommandDialog();
	ResetContents();
}

void KeywordManagerDialog::OnButtonEdit()
{
	if (in->GetSelectedCommands().size() != 1) {
		return;
	}

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
	if (in->GetSelectedCommands().size() != 1) {
		return;
	}

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
	auto selectedCommands = in->GetSelectedCommands();
	if (selectedCommands.empty()) {
		return;
	}

	std::vector<Command*> commandsToDelete;
	std::vector<Command*> commandsToKeepSelected;
	for (auto command : selectedCommands) {
		if (in->IsDeletable(command)) {
			commandsToDelete.push_back(command);
		}
		else {
			commandsToKeepSelected.push_back(command);
		}
	}
	if (commandsToDelete.empty()) {
		return;
	}

	CString confirmMsg((LPCTSTR)IDS_CONFIRM_DELETE);
	confirmMsg += _T("\n");
	confirmMsg += _T("\n");
	for (auto command : commandsToDelete) {
		confirmMsg += command->GetName();
		confirmMsg += _T("\n");
	}

	int sel = AfxMessageBox(confirmMsg, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2);
	if (sel != IDYES) {
		return ;
	}

	in->SelectItems({}, false);
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	for (auto command : commandsToDelete) {
		cmdRepoPtr->UnregisterCommand(command);
	}

	ResetContents();
	in->SelectItems(commandsToKeepSelected, true);
	UpdateStatus();
	UpdateData(FALSE);
}

void KeywordManagerDialog::OnButtonImport()
{
	CFileDialog dlg(TRUE, _T("ini"), nullptr, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST,
	                _T("INIファイル (*.ini)|*.ini||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	CommandFile commandFile;
	commandFile.SetFilePath(dlg.GetPathName());
	if (commandFile.Load() == false) {
		spdlog::error(_T("Failed to load command import file. path:{}"), (LPCTSTR)dlg.GetPathName());
		AfxMessageBox(_T("ファイルを読み込めませんでした。"), MB_OK | MB_ICONERROR);
		return;
	}

	struct ImportCandidate {
		RefPtr<Command> mCommand;
		RefPtr<UserCommandProvider> mProvider;
		int mEntryIndex{0};
	};

	auto providerRepository = CommandProviderRepository::GetInstance();
	std::vector<CommandProvider*> providers;
	providerRepository->EnumProviders(providers);

	std::vector<ImportCandidate> candidates;
	std::vector<CString> skippedEntryNames;
	for (int entryIndex = 0; entryIndex < commandFile.GetEntryCount(); ++entryIndex) {
		auto entry = commandFile.GetEntry(entryIndex);
		ImportCandidate candidate;
		candidate.mEntryIndex = entryIndex;

		for (auto provider : providers) {
			RefPtr<UserCommandProvider> userProvider;
			if (provider->QueryInterface(IFID_USERCOMMANDPROVIDER, (void**)&userProvider) == false) {
				continue;
			}

			RefPtr<Command> command;
			if (userProvider->LoadFrom(entry, &command) == false || command.get() == nullptr) {
				command.reset();
				continue;
			}

			candidate.mCommand.swap(command);
			candidate.mProvider.swap(userProvider);
			commandFile.MarkAsUsed(entry);
			break;
		}

		if (candidate.mCommand.get() == nullptr) {
			skippedEntryNames.push_back(commandFile.GetName(entry));
			continue;
		}
		candidates.push_back(std::move(candidate));
	}

	if (skippedEntryNames.empty() == false) {
		CString warning;
		warning.Format(_T("読み込めない、または未対応のエントリを %d 件除外しました。\n\n"), (int)skippedEntryNames.size());
		for (auto& name : skippedEntryNames) {
			warning += name;
			warning += _T("\n");
		}
		AfxMessageBox(warning, MB_OK | MB_ICONWARNING);
	}

	if (candidates.empty()) {
		if (skippedEntryNames.empty()) {
			AfxMessageBox(_T("インポートできるコマンドがありません。"), MB_OK | MB_ICONWARNING);
		}
		return;
	}

	std::vector<Command*> candidateCommands;
	candidateCommands.reserve(candidates.size());
	for (auto& candidate : candidates) {
		candidateCommands.push_back(candidate.mCommand.get());
	}

	ImportCommandsDialog importDialog;
	importDialog.SetCommands(candidateCommands);
	if (importDialog.DoModal() != IDOK) {
		return;
	}

	auto cmdRepoPtr = CommandRepository::GetInstance();
	auto hotKeyManager = CommandHotKeyManager::GetInstance();
	std::vector<CString> importedNames;
	for (auto candidateIndex : importDialog.GetSelectedIndices()) {
		if (candidateIndex < 0 || candidateIndex >= (int)candidates.size()) {
			continue;
		}

		auto& candidate = candidates[candidateIndex];
		RefPtr<Command> command(candidate.mCommand);
		CString commandName = command->GetName();
		RefPtr<Command> existingCommand(cmdRepoPtr->QueryAsWholeMatch(commandName));

		if (existingCommand.get() != nullptr && importDialog.IsOverwriteSelected()) {
			CString existingName = existingCommand->GetName();
			CommandHotKeyMappings previousMappings;
			hotKeyManager->GetMappings(previousMappings);

			CommandHotKeyAttribute previousHotKey;
			bool hasPreviousHotKey = false;
			for (int index = 0; index < previousMappings.GetItemCount(); ++index) {
				if (previousMappings.GetName(index) != existingName) {
					continue;
				}
				previousMappings.GetHotKeyAttr(index, previousHotKey);
				hasPreviousHotKey = true;
				break;
			}

			cmdRepoPtr->UnregisterCommand(existingCommand.get());
			command->AddRef();
			cmdRepoPtr->RegisterCommand(command.get());

			CommandHotKeyMappings currentMappings;
			hotKeyManager->GetMappings(currentMappings);
			bool hasChanged = currentMappings.RemoveItem(commandName);
			if (hasPreviousHotKey) {
				currentMappings.AddItem(commandName, previousHotKey);
				hasChanged = true;
			}
			if (hasChanged) {
				auto preference = AppPreference::Get();
				preference->SetCommandKeyMappings(currentMappings);
				preference->Save();
			}
		}
		else {
			if (existingCommand.get() != nullptr) {
				CString uniqueName = launcherapp::core::CommandImportNameResolver::GetUniqueName(commandName, [&](const CString& name) {
					RefPtr<Command> existing(cmdRepoPtr->QueryAsWholeMatch(name));
					return existing.get() != nullptr;
				});

				auto entry = static_cast<CommandFileEntry*>(commandFile.GetEntry(candidate.mEntryIndex));
				entry->SetName(uniqueName);
				RefPtr<Command> renamedCommand;
				if (candidate.mProvider->LoadFrom(entry, &renamedCommand) == false || renamedCommand.get() == nullptr) {
					spdlog::error(_T("Failed to reload command with an import name. name:{}"), (LPCTSTR)uniqueName);
					continue;
				}
				command.swap(renamedCommand);
				commandName = command->GetName();
			}

			command->AddRef();
			cmdRepoPtr->RegisterCommand(command.get());
		}

		if (std::none_of(importedNames.begin(), importedNames.end(), [&](const CString& name) {
			return name.CompareNoCase(commandName) == 0;
		})) {
			importedNames.push_back(commandName);
		}
	}

	if (importedNames.empty()) {
		return;
	}

	ResetContents();
	std::vector<Command*> importedCommands;
	std::vector<RefPtr<Command>> importedCommandRefs;
	for (auto& name : importedNames) {
		RefPtr<Command> command(cmdRepoPtr->QueryAsWholeMatch(name));
		if (command.get() == nullptr) {
			continue;
		}
		importedCommands.push_back(command.get());
		importedCommandRefs.push_back(std::move(command));
	}
	in->SelectItems(importedCommands, true);
	UpdateStatus();
	UpdateData(FALSE);
}

void KeywordManagerDialog::OnButtonExport()
{
	auto selectedCommands = in->GetSelectedCommands();
	selectedCommands.erase(std::remove_if(selectedCommands.begin(), selectedCommands.end(), [&](Command* command) {
		return in->IsUserCommand(command) == false;
	}), selectedCommands.end());
	if (selectedCommands.empty()) {
		return;
	}

	CFileDialog dlg(FALSE, _T("ini"), nullptr, OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST,
	                _T("INIファイル (*.ini)|*.ini||"), this);
	if (dlg.DoModal() != IDOK) {
		return;
	}

	CommandFile commandFile;
	commandFile.SetFilePath(dlg.GetPathName());
	for (auto command : selectedCommands) {
		auto entry = commandFile.NewEntry(command->GetName());
		if (command->Save(entry) == false) {
			spdlog::error(_T("Failed to save command for export. name:{}"), (LPCTSTR)command->GetName());
			CString message;
			message.Format(_T("コマンド %s の保存に失敗しました。"), (LPCTSTR)command->GetName());
			AfxMessageBox(message, MB_OK | MB_ICONERROR);
			return;
		}
	}

	if (commandFile.Save() == false) {
		spdlog::error(_T("Failed to save exported command file. path:{}"), (LPCTSTR)dlg.GetPathName());
		AfxMessageBox(_T("ファイルの保存に失敗しました。"), MB_OK | MB_ICONERROR);
	}
}

void KeywordManagerDialog::OnEditCopy()
{
	std::vector<RefPtr<CommandEntryIF>> commandEntries;
	for (auto command : in->GetSelectedCommands()) {
		if (in->IsUserCommand(command) == false) {
			continue;
		}

		auto transfer = launcherapp::commands::transfer::CommandClipboardTransfer::GetInstance();
		RefPtr<CommandEntryIF> entry(transfer->NewEntry(command->GetName()));
		if (command->Save(entry.get()) == false) {
			spdlog::error(_T("Failed to save command.{}"), (LPCTSTR)command->GetName());
			return;
		}
		commandEntries.push_back(std::move(entry));
	}
	if (commandEntries.empty()) {
		return;
	}

	auto transfer = launcherapp::commands::transfer::CommandClipboardTransfer::GetInstance();
	std::vector<CommandEntryIF*> entries;
	for (auto& entry : commandEntries) {
		entries.push_back(entry.get());
	}
	transfer->SendEntries(entries);
}

void KeywordManagerDialog::OnEditPaste()
{
	auto transfer = launcherapp::commands::transfer::CommandClipboardTransfer::GetInstance();

	std::vector<RefPtr<CommandEntryIF>> entries;
	if (transfer->ReceiveEntries(entries) == false || entries.empty()) {
		return;
	}

	auto providerRepos = launcherapp::core::CommandProviderRepository::GetInstance();

	std::vector<launcherapp::core::CommandProvider*> providers;
	providerRepos->EnumProviders(providers);

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	for (auto& entry : entries) {
		RefPtr<Command> newCmd;
		bool isLoaded = false;
		for (auto provider : providers) {
			RefPtr<launcherapp::core::UserCommandProvider> userCmdProvider;
			if (provider->QueryInterface(IFID_USERCOMMANDPROVIDER, (void**)&userCmdProvider) == false) {
				continue;
			}
			if (userCmdProvider->LoadFrom(entry.get(), &newCmd)) {
				isLoaded = true;
				break;
			}
			newCmd.reset();
		}
		if (isLoaded == false || newCmd.get() == nullptr) {
			spdlog::error(_T("Failed to load copied command.{}"), (LPCTSTR)entry->GetName());
			return;
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
		UpdateStatus();
		UpdateData(FALSE);
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

