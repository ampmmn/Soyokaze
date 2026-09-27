#include "pch.h"
#include "framework.h"
#include "features/keywordmanager/KeywordManagerDialog.h"
#include "control/CommandListCtrl.h"
#include "control/KeywordEdit.h"
#include "icon/IconLabel.h"
#include "core/IFIDDefine.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "features/keywordmanager/CommandImportExport.h"
#include "commands/core/UserCommandProvider.h"
#include "commands/core/EditableIF.h"
#include "commands/core/CommandProviderRepository.h"
#include "commands/transfer/CommandClipboardTransfer.h"
#include "features/keywordmanager/ImportCommandsDialog.h"
#include "utility/RefPtr.h"
#include "hotkey/CommandHotKeyManager.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <algorithm>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr UINT TIMERID_FILTER = 1;

using namespace launcherapp::core;

struct KeywordManagerDialog::PImpl : public CommandRepositoryListenerIF
{
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

	Command* mSelCommand{nullptr};

	CommandHotKeyMappings mKeyMapping;
	CommandListCtrl mListCtrl;
	std::unique_ptr<IconLabel> mIconLabelPtr;
	KeywordEdit mKeywordEdit;

	HWND mHwnd{nullptr};
	HACCEL mAccel{nullptr};

	// フィルター更新タイマーID
	UINT_PTR mUpdateTimerId{0};
};

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
	in->mListCtrl.SetColumnMode(CommandListCtrl::ColumnMode::WithHotKey);
	in->mListCtrl.SetSelectionMode(CommandListCtrl::SelectionMode::Multiple);
	in->mListCtrl.SetHotKeyMappings(&in->mKeyMapping);
	in->mListCtrl.Initialize();
	in->mKeywordEdit.SubclassDlgItem(IDC_EDIT_FILTER, this);
	in->mIconLabelPtr->SubclassDlgItem(IDC_STATIC_ICON, this);
	in->mIconLabelPtr->DrawIcon(IconLoader::Get()->LoadKeywordManagerIcon());

	// フィルタ欄にプレースホルダーを設定する
	in->mKeywordEdit.SetPlaceHolder(_T("文字列をここに入力するとリストの絞り込みができます"));

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
	// コマンド一覧を取得する
	std::vector<Command*> commands;
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	cmdRepoPtr->EnumCommands(commands);

	// ホットキー一覧を取得
	CommandHotKeyManager::GetInstance()->GetMappings(in->mKeyMapping);
	in->mListCtrl.SetHotKeyMappings(&in->mKeyMapping);
	in->mListCtrl.SetCommands(commands);
	for (auto command : commands) {
		command->Release();
	}
}

bool KeywordManagerDialog::UpdateStatus()
{
	auto selectedCommands = in->mListCtrl.GetSelectedCommands();
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
	in->mListCtrl.SetFilterText(in->mFilterStr);
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
	if (in->mListCtrl.GetSelectedCommands().size() != 1) {
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
	if (in->mListCtrl.GetSelectedCommands().size() != 1) {
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
	auto selectedCommands = in->mListCtrl.GetSelectedCommands();
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

	in->mListCtrl.SelectCommands({}, false);
	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	for (auto command : commandsToDelete) {
		cmdRepoPtr->UnregisterCommand(command);
	}

	ResetContents();
	in->mListCtrl.SelectCommands(commandsToKeepSelected, true);
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

	CommandImportExport commandImportExport;
	if (commandImportExport.LoadCommands(dlg.GetPathName()) == false) {
		spdlog::error(_T("Failed to load command import file. path:{}"), (LPCTSTR)dlg.GetPathName());
		AfxMessageBox(_T("ファイルを読み込めませんでした。"), MB_OK | MB_ICONERROR);
		return;
	}

	const auto& skippedEntryNames = commandImportExport.GetSkippedEntryNames();

	if (skippedEntryNames.empty() == false) {
		CString warning;
		warning.Format(_T("読み込めない、または未対応のエントリを %d 件除外しました。\n\n"), (int)skippedEntryNames.size());
		for (auto& name : skippedEntryNames) {
			warning += name;
			warning += _T("\n");
		}
		AfxMessageBox(warning, MB_OK | MB_ICONWARNING);
	}

	const auto& candidateCommands = commandImportExport.GetImportCandidates();
	if (candidateCommands.empty()) {
		if (skippedEntryNames.empty()) {
			AfxMessageBox(_T("インポートできるコマンドがありません。"), MB_OK | MB_ICONWARNING);
		}
		return;
	}

	ImportCommandsDialog importDialog;
	importDialog.SetCommands(candidateCommands);
	if (importDialog.DoModal() != IDOK) {
		return;
	}

	auto cmdRepoPtr = CommandRepository::GetInstance();
	auto importedNames = commandImportExport.ImportCommands(importDialog.GetSelectedIndices(), importDialog.IsOverwriteSelected());

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
	in->mListCtrl.SelectCommands(importedCommands, true);
	UpdateStatus();
	UpdateData(FALSE);
}

void KeywordManagerDialog::OnButtonExport()
{
	auto selectedCommands = in->mListCtrl.GetSelectedCommands();
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

	CommandImportExport commandImportExport;
	auto result = commandImportExport.ExportCommands(selectedCommands, dlg.GetPathName());
	if (result.mError == CommandImportExport::ExportError::CommandSaveFailed) {
		spdlog::error(_T("Failed to save command for export. name:{}"), (LPCTSTR)result.mCommandName);
		CString message;
		message.Format(_T("コマンド %s の保存に失敗しました。"), (LPCTSTR)result.mCommandName);
		AfxMessageBox(message, MB_OK | MB_ICONERROR);
	}
	else if (result.mError == CommandImportExport::ExportError::FileSaveFailed) {
		spdlog::error(_T("Failed to save exported command file. path:{}"), (LPCTSTR)dlg.GetPathName());
		AfxMessageBox(_T("ファイルの保存に失敗しました。"), MB_OK | MB_ICONERROR);
	}
}

void KeywordManagerDialog::OnEditCopy()
{
	std::vector<RefPtr<CommandEntryIF>> commandEntries;
	for (auto command : in->mListCtrl.GetSelectedCommands()) {
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
		in->mListCtrl.SelectCommand(newCmd.get(), false);
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

LRESULT KeywordManagerDialog::OnUserMessageKeywrodEditKeyDown(WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);

	// 矢印↓キー押下
	if (wParam ==VK_DOWN) {
		in->mListCtrl.SetFocus();

		if (in->mListCtrl.GetVisibleCommandCount() > 0) {
			in->mListCtrl.SelectItemAt(0, false);
		}
		return 1;
	}
	if (wParam ==VK_UP) {
		in->mListCtrl.SetFocus();

		if (in->mListCtrl.GetVisibleCommandCount() > 0) {
			int visibleItems = in->mListCtrl.GetVisibleCommandCount();
			in->mListCtrl.SelectItemAt(visibleItems - 1, false);
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

