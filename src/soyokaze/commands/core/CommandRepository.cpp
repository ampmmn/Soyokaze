#include "pch.h"
#include "CommandRepository.h"
#include "commands/core/CommandFileEntry.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandMap.h"
#include "commands/core/CommandQueryRequest.h"
#include "commands/core/DefaultCommand.h"
#include "commands/core/IFIDDefine.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include "matcher/PartialMatchPattern.h"
#include "matcher/WholeMatchPattern.h"
#include "commands/core/CommandProviderIF.h"
#include "commands/core/CommandFile.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "gui/KeywordManagerDialog.h"
#include "gui/SelectFilesDialog.h"
#include "gui/SelectCommandTypeDialog.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/core/CommandRanking.h"
#include "commands/common/CommandParameterFunctions.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "hotkey/NamedCommandHotKeyHandler.h"
#include "spdlog/stopwatch.h"
#include <vector>
#include <algorithm>
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace core {

using namespace launcherapp::commands::common;

using ShellExecCommand  = launcherapp::commands::shellexecute::ShellExecCommand;
using DefaultCommand = launcherapp::commands::core::DefaultCommand;
using CommandRanking = launcherapp::commands::core::CommandRanking;
using QueryRequest = launcherapp::commands::core::CommandQueryRequest;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct CommandRepository::PImpl
{
	void ReloadPatternObject()
	{
		SPDLOG_DEBUG(_T("start"));

		mPattern.reset(PartialMatchPattern::Create());

		// コマンドのホットキー設定のリロード
		CommandHotKeyMappings hotKeyMap;

		auto pref = AppPreference::Get();
		pref->GetCommandKeyMappings(hotKeyMap);

		auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
		hotKeyManager->Clear(this);

		int count = hotKeyMap.GetItemCount();
		SPDLOG_DEBUG(_T("hotKeyMap number of items:{}"), count);
		for (int i = 0; i < count; ++i) {
			auto name = hotKeyMap.GetName(i);
			CommandHotKeyAttribute attr;
			hotKeyMap.GetHotKeyAttr(i, attr);

			auto handler = std::make_unique<NamedCommandHotKeyHandler>(name);

			hotKeyManager->Register(this, handler.release(), attr);
		}
		SPDLOG_DEBUG(_T("end"));
	}

	void SaveCommands()
	{
		SPDLOG_DEBUG(_T("start"));

		CommandFile commandFile;
		commandFile.SetFilePath(mCommandFilePath);

		CString name;

		std::vector<launcherapp::core::Command*> commands;
		for (auto& command : mCommands.Enumerate(commands)) {

			name = command->GetName();
			auto entry = commandFile.NewEntry(name);
			command->Save(entry);
			command->Release();
		}
		commandFile.Save();
	}

	void RunQueryThread();

	bool IsRunning() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsExit == false;
	}
	void SetExit()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsExit = true;
		mQueryRequestEvt.SetEvent();
	}
	void AddRequest(const QueryRequest& req);
	bool WaitForRequest(QueryRequest& req);
	bool HasSubsequentRequest();
	void Query(QueryRequest& req);

	DefaultCommand* GetDefaultCommand()
	{
		if (mDefaultCommand.get() == nullptr) {
			mDefaultCommand.reset(new DefaultCommand());
		}
		return mDefaultCommand.get();
	}

	CCriticalSection mCS;

	std::vector<CommandProvider*> mProviders;

	// リスナー
	std::vector<CommandRepositoryListenerIF*> mListeners;

	// 設定ファイル(commands.ini)のパス
	CString mCommandFilePath;

	// 一般コマンド一覧
	CommandMap mCommands;
	// キーワード比較用のクラス
	RefPtr<Pattern> mPattern;

	// 一致したコマンドがなかったときのコマンド
	std::unique_ptr<DefaultCommand> mDefaultCommand;

	
	// 編集中フラグ
	bool mIsNewDialog = false;
	bool mIsEditDialog = false;
	KeywordManagerDialog* mManagerDlgPtr = nullptr;
	bool mIsRegisteFromFileDialog = false;

	//

	std::mutex mMutex;
	bool mIsExit = false;
	bool mIsThreadInitialized = false;
	CEvent mQueryRequestEvt;
	std::vector<QueryRequest> mQueryRequestQueue;
};

void CommandRepository::PImpl::RunQueryThread()
{
	std::lock_guard<std::mutex> lock(mMutex);
	if (mIsExit) {
		return ;
	}
	if (mIsThreadInitialized) {
		return ;
	}

	std::thread th([&]() {

			spdlog::info("Enter RunQueryThread.");

			while (IsRunning()) {
				QueryRequest req;
				if (WaitForRequest(req) == false) {
					continue;
				}
				Query(req);
			}
			spdlog::info("Exit RunQueryThread.");

	});
	th.detach();

	mIsThreadInitialized = true;
}

void CommandRepository::PImpl::AddRequest(const QueryRequest& req)
{
	{
		PERFLOG("AddRequest lock start.");
		spdlog::stopwatch sw;

		std::lock_guard<std::mutex> lock(mMutex);
		mQueryRequestQueue.push_back(req);

		PERFLOG("AddRequest lock end. {0:.6f} s.", sw);
	}
	mQueryRequestEvt.SetEvent();
}

bool CommandRepository::PImpl::WaitForRequest(QueryRequest& req)
{
	PERFLOG("WaitForRequest wait start.");
	spdlog::stopwatch sw;
	WaitForSingleObject(mQueryRequestEvt, INFINITE);
	PERFLOG("WaitForRequest wait end. {0:.6f} s.", sw);

	std::lock_guard<std::mutex> lock(mMutex);
	if (mIsExit) {
		return false;
	}
	if (mQueryRequestQueue.empty()) {
		return false;
	}

	req = mQueryRequestQueue.back();
	mQueryRequestQueue.clear();

	return true;
}

bool CommandRepository::PImpl::HasSubsequentRequest()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return mQueryRequestQueue.size() > 0;
}

void CommandRepository::PImpl::Query(QueryRequest& req)
{
	const auto param = req.GetCommandParameter();
	HWND hwndNotify = req.GetNotifyWindow();
	UINT notifyMsg = req.GetNotifyMessage();

	// パラメータが空の場合は検索しない
	if (param->IsEmpty()) {
		// 結果なし
		PostMessage(hwndNotify, notifyMsg, 0, 0);
		return;
	}

	CSingleLock sl(&mCS, TRUE);

	spdlog::stopwatch swAll;

	// 入力文字列をを設定
	mPattern->SetWholeText(param->GetWholeString());

	CommandMap::CommandQueryItemList matchedItems;

	spdlog::stopwatch sw;
	PERFLOG("CommandMap.Query start.");
	mCommands.Query(mPattern.get(), matchedItems);
	PERFLOG("CommandMap.Query end. {0:.6f} s num:{1}", sw, (int)matchedItems.GetItemCount());
	  // Note: ここで+1した参照カウントは CommandRepository::Query 呼び出し元で-1する必要あり

	// コマンドプロバイダーから一時的なコマンドを取得する
	int prevCount = (int)matchedItems.GetItemCount();
	for (auto& provider : mProviders) {
		sw.reset();
		if (HasSubsequentRequest()) {
			// キャンセル通知
			PostMessage(hwndNotify, notifyMsg, 1, 0);
			break;
		}
		PERFLOG(_T("QueryAdhocCommands start. name:{0}"), (LPCTSTR)provider->GetName());
		provider->QueryAdhocCommands(mPattern.get(), matchedItems);

		int n = (int)matchedItems.GetItemCount();
		PERFLOG(_T("QueryAdhocCommands end. name:{0} {1:.6f} s num:{2}"), (LPCTSTR)provider->GetName(), sw.elapsed().count(), n - prevCount);
		prevCount = n;
	}


	sw.reset();

	// 一致レベルに基づきソート
	int itemCount = (int)matchedItems.GetItemCount();
	PERFLOG("sort candidates start num:{0}", itemCount);
	matchedItems.Sort();
	PERFLOG("sort candidates end {1:.6f} s.", sw);

	std::vector<launcherapp::core::Command*>* items = new std::vector<launcherapp::core::Command*>();
	
	if (itemCount > 0) {
		items->resize(itemCount);
		matchedItems.GetItems(&(items->front()), items->size());
	}
	else {
		auto defaultCmd = GetDefaultCommand();
		defaultCmd->SetName(param->GetWholeString());
		items->push_back(defaultCmd);
	}

	PERFLOG("Query total processsing time: {:.6f} s.", swAll);

	PostMessage(hwndNotify, notifyMsg, 0, (LPARAM)items);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandRepository::CommandRepository() : in(std::make_unique<PImpl>())
{
	AppPreference::Get()->RegisterListener(this);

	Path path(Path::APPDIR, _T("commands.ini"));
	in->mCommandFilePath = path;
}

CommandRepository::~CommandRepository()
{
}

// コマンドプロバイダ登録
void CommandRepository::RegisterProvider(
	CommandProvider* provider
)
{
	CSingleLock sl(&in->mCS, TRUE);

	provider->AddRef();
	in->mProviders.push_back(provider);
	std::sort(in->mProviders.begin(), in->mProviders.end(),
	          [](const CommandProvider* l, const CommandProvider* r) { return l->GetOrder() < r->GetOrder(); });
}

/**
 	コマンドプロバイダの設定ページを列挙する
	@remarks 取得したインスタンスは呼び出し側で解放すること
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  SettingPagesの一覧
*/
void CommandRepository::EnumProviderSettingDialogs(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	CSingleLock sl(&in->mCS, TRUE);

	for (auto& provider : in->mProviders) {
		provider->CreateSettingPages(parent, pages);
	}
}


// コマンドを登録
// このコマンドは参照カウントを+1しない
int CommandRepository::RegisterCommand(Command* command, bool isNotify)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Register(command);

	// コマンドが登録されたことをリスナーに通知
	for (auto& listener : in->mListeners) {
		listener->OnNewCommand(command);
	}

	// ホットキーの登録
	CommandHotKeyAttribute hotKeyAttr;
	if (command->GetHotKeyAttribute(hotKeyAttr)) {
		if (hotKeyAttr.IsValid() || hotKeyAttr.IsValidSandS()) {

			auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();

			// マネージャーから管理しているコマンドホットキーの一覧を取得
			CommandHotKeyMappings hotKeyMap;
			hotKeyManager->GetMappings(hotKeyMap);

			// 取得したホットキーの一覧をAppPreferenceに設定
			auto pref = AppPreference::Get();
			pref->SetCommandKeyMappings(hotKeyMap);

			if (isNotify) {
				// このタイミングで設定ファイルに反映される
				// Note: 保存時の通知を通じて、CommandRepository::ReloadPatternObject内でホットキーのリロードを行う
				pref->Save();
			}
		}
	}

	return 0;
}

// コマンドの登録を解除
int CommandRepository::UnregisterCommand(Command* command)
{
	CSingleLock sl(&in->mCS, TRUE);

	// コマンドが削除されようとしていることをリスナーに通知
	for (auto& listener : in->mListeners) {
		listener->OnDeleteCommand(command);
	}

	auto name = command->GetName();

	// ホットキーの登録解除
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();

	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);
	bool isRemoved = hotKeyMap.RemoveItem(name);

	if (isRemoved) {
		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		// Note: 保存時の通知を通じて、CommandRepository::ReloadPatternObject内でホットキーのリロードを行う
		pref->Save();
	}

	CommandRanking::GetInstance()->Delete(command);
	in->mCommands.Unregister(command);
	return 0;
}

// 名前変更による登録しなおし
int CommandRepository::ReregisterCommand(Command* command)
{
	CSingleLock sl(&in->mCS, TRUE);

	// 再登録
	in->mCommands.Reregister(command);

	// ホットキーの再登録
	CommandHotKeyAttribute hotKeyAttr;
	if (command->GetHotKeyAttribute(hotKeyAttr)) {

		auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();

		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		// 以前の設定を消して、新しい設定を登録する
		auto name = command->GetName();
		hotKeyMap.RemoveItem(name);
		if (hotKeyAttr.IsValid() || hotKeyAttr.IsValidSandS()) {
			hotKeyMap.AddItem(name, hotKeyAttr);
		}

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		// Note: 保存時の通知を通じて、CommandRepository::ReloadPatternObject内でホットキーのリロードを行う
		pref->Save();
	}

	return 0;
}

CommandRepository* CommandRepository::GetInstance()
{
	static CommandRepository inst;
	return &inst;
}

BOOL CommandRepository::Load()
{
	SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&in->mCS, TRUE);
	// 既存の内容を破棄
	in->mCommands.Clear();

	// キーワード比較処理の生成
	in->ReloadPatternObject();

	auto tmpProviders = in->mProviders;
	sl.Unlock();

	// ロード前を通知するイベントをリスナーに通知する
	for (auto& listener : in->mListeners) {
		listener->OnBeforeLoad();
	}

	// 設定ファイルを読み、コマンド一覧を登録する
	CommandFile commandFile;
	commandFile.SetFilePath(in->mCommandFilePath);
	commandFile.Load();

	for (auto& provider : tmpProviders) {
		provider->LoadCommands(&commandFile);
	}

	CommandRanking::GetInstance()->Load();

	return TRUE;
}


class ScopeEdit
{
public:
	ScopeEdit(bool& flag) : mFlagPtr(&flag)
 	{
		*mFlagPtr = true;
	}
	~ScopeEdit()
	{
		*mFlagPtr = false;
	}
	bool* mFlagPtr;
};


/**
 *  新規キーワード作成
 *  @param paramr パラメータ
 */
int CommandRepository::NewCommandDialog(CommandParameter* param)
{
	if (in->mIsNewDialog) {
		// 編集操作中の再入はしない
		SPDLOG_WARN(_T("re-entry is not allowed."));
		return -1;
	}
	ScopeEdit scopeEdit(in->mIsNewDialog);

	CommandProvider* selectedProvider = nullptr;

	// 種類選択ダイアログを表示
	CString typeStr;
	if (param) {
		auto namedParam = GetCommandNamedParameter(param);
		int len = namedParam->GetNamedParamStringLength(_T("TYPE"));
		if (len > 0) {
			namedParam->GetNamedParamString(_T("TYPE"), typeStr.GetBuffer(len), len);
			typeStr.ReleaseBuffer();
		}
	}

	if (typeStr.IsEmpty()) {
		SelectCommandTypeDialog dlgSelect;

		for (auto& provider : in->mProviders) {
			if (provider->IsPrivate()) {
				continue;
			}

			dlgSelect.AddType(provider->GetDisplayName(), provider->GetDescription(), (LPARAM)provider);
		}

		if (dlgSelect.DoModal() != IDOK) {
			return 0;
		}
		selectedProvider = (CommandProvider*)dlgSelect.GetSelectedItem();
	}
	else {
		for (auto& provider : in->mProviders) {
			if (typeStr != provider->GetName()) {
				continue;
			}
			selectedProvider = provider;
			break;
		}
	}

	if (selectedProvider == nullptr) {
		return 0;
	}

	if (selectedProvider->NewDialog(param) == false) {
		return 1;
	}

	// コマンド設定ファイルに保存
	in->SaveCommands();

	return 0;

	
}

/**
 *  既存キーワードの編集
 */
int CommandRepository::EditCommandDialog(const CString& cmdName, bool isClone)
{
	if (in->mIsEditDialog) {
		// 編集操作中の再入はしない
		SPDLOG_WARN(_T("re-entry is not allowed."));
		return 0;
	}
	ScopeEdit scopeEdit(in->mIsEditDialog);

	auto cmdAbs = in->mCommands.Get(cmdName);
	if (cmdAbs == nullptr) {
		return 1;
	}

	HWND parent = nullptr;
	if (in->mManagerDlgPtr) {
		parent = in->mManagerDlgPtr->GetSafeHwnd();
	}

	RefPtr<Editable> editable;
	if (cmdAbs->QueryInterface(IFID_EDITABLE, (void**)&editable) == false || 
      editable->IsEditable() == false) {
		spdlog::info(_T("{} is unediable."), (LPCTSTR)cmdAbs->GetName());
		return 2;
	}

	RefPtr<CommandEditor> editor;
	if (editable->CreateEditor(parent, &editor) == false) {
		spdlog::error(_T("Failed to create editor. name:{} "), (LPCTSTR)cmdAbs->GetName());
		return 3;
	}

	if (isClone == false) {
		// 変更前の名前をセットする
		editor->SetOriginalName(cmdAbs->GetName());
		// 設定画面を表示する
		if (editor->DoModal() == false) {
			spdlog::info(_T("edit cancelled. name:{}"), (LPCTSTR)cmdAbs->GetName());
			return 0;
		}

		// 設定画面上の変更をコマンドに適用する
		if (editable->Apply(editor.get()) == false) {
			spdlog::error(_T("Failed to apply. name:{} "), (LPCTSTR)cmdAbs->GetName());
			return 4;
		}
		// コマンドを再登録(名前変更があった場合にそれを反映する)
		ReregisterCommand(cmdAbs);
	}
	else {
		// 元コマンドの名前をベースに新しい名前を設定する
		editor->OverrideName(IssueClonedCommandName(cmdAbs->GetName()));
		// 設定画面を表示する
		if (editor->DoModal() == false) {
			spdlog::info(_T("clone cancelled. name:{}"), (LPCTSTR)cmdAbs->GetName());
			return 0;
		}

		// 設定画面上の変更をコマンドに適用する
		RefPtr<launcherapp::core::Command> newCmd;
		if (editable->CreateNewInstanceFrom(editor.get(), &newCmd) == false) {
			spdlog::error(_T("Failed to clone. name:{} "), (LPCTSTR)cmdAbs->GetName());
			return 4;
		}

		// 複製したコマンドを登録する
		constexpr bool isReloadHotKey = true;
		RegisterCommand(newCmd.release(), isReloadHotKey);
	}

	// コマンドファイルに保存
	in->SaveCommands();

	return 0;
}

/**
 *  キーワードマネージャーの表示
 */
int CommandRepository::ManagerDialog()
{
	if (in->mManagerDlgPtr != nullptr) {
		// 編集操作中の再入はしない
		SPDLOG_WARN(_T("re-entry is not allowed."));
		in->mManagerDlgPtr->SetForegroundWindow();
		return 0;
	}

	struct scope_clear {
		scope_clear(KeywordManagerDialog*& p) : ptr(p) {}
		~scope_clear() { ptr = nullptr; }
		KeywordManagerDialog*& ptr;
	} _scope(in->mManagerDlgPtr);


	// キャンセル時用のバックアップ
	CommandMap::Settings bkup;
	in->mCommands.LoadSettings(bkup);

	KeywordManagerDialog dlg;
	in->mManagerDlgPtr = &dlg;
	if (dlg.DoModal() != IDOK) {

		// OKではないので結果を反映しない(バックアップした内容に戻す)
		in->mCommands.RestoreSettings(bkup);
	}

	// ファイルに保存
	in->SaveCommands();

	return 0;
}


// まとめて登録ダイアログの表示
int CommandRepository::RegisterCommandFromFiles(
	const std::vector<CString>& files
)
{
	if (in->mIsRegisteFromFileDialog) {
		// 編集操作中の再入はしない
		SPDLOG_WARN(_T("re-entry is not allowed."));
		return 0;
	}

	ScopeEdit scopeEdit(in->mIsRegisteFromFileDialog);

	if (files.empty()) {
		// ファイルパスの要素数が0の場合はなにもしない
		return 0;
	}

	if (files.size() > 1) {
		// 複数の場合はまとめて選択ダイアログ
		SelectFilesDialog dlg;
		dlg.SetFiles(files);

		if (dlg.DoModal() != IDOK) {
			return 0;
		}

		std::vector<CString> filesToRegister;
		dlg.GetCheckedFiles(filesToRegister);

		if (filesToRegister.empty()) {
			return 0;
		}

		// ダイアログで選択されたファイルを一括登録
		for (const auto& filePath : filesToRegister) {
			ShellExecCommand::NewCommand(filePath);
		}

		AfxMessageBox(_T("登録しました"));
	}
	else {
		// 登録ダイアログを表示

		CString filePath = files[0];

		CString name(PathFindFileName(filePath));
		PathRemoveExtension(name.GetBuffer(name.GetLength()));
		name.ReleaseBuffer();

		RefPtr<CommandParameterBuilder> param(CommandParameterBuilder::Create(), false);
		param->SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param->SetNamedParamString(_T("COMMAND"), name);
		param->SetNamedParamString(_T("PATH"), filePath);

		auto result = NewCommandDialog(param);
		return result;
	}
	return 0;
}

void CommandRepository::EnumCommands(std::vector<launcherapp::core::Command*>& enumCommands)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Enumerate(enumCommands);
}


void CommandRepository::Activate()
{
	// ランチャーがアクティブになった通知
	for (auto& listener : in->mListeners) {
		listener->OnLancuherActivate();
	}
}

void CommandRepository::Unactivate()
{
	// ランチャーが非アクティブになった通知
	for (auto& listener : in->mListeners) {
		listener->OnLancuherUnactivate();
	}
}

void
CommandRepository::Query(
	const QueryRequest& newRequest
)
{
	// 問い合わせリクエストをキューに追加
	in->AddRequest(newRequest);
}

launcherapp::core::Command*
CommandRepository::QueryAsWholeMatch(
	const CString& strQueryStr,
	bool isIncludeAdhocCommand
)
{
	CSingleLock sl(&in->mCS, TRUE);

	// パラメータが空の場合は検索しない
	if (strQueryStr.IsEmpty()) {
		return nullptr;
	}

	RefPtr<WholeMatchPattern> pat(WholeMatchPattern::Create(strQueryStr));

	auto command = in->mCommands.FindOne(pat.get());
	if (command != nullptr) {
		return command;
	}

	if (isIncludeAdhocCommand == false) {
		return nullptr;
	}

	// コマンドプロバイダーから一時的なコマンドを取得する
	launcherapp::CommandQueryItemList matchedItems;
	for (auto& provider : in->mProviders) {
		provider->QueryAdhocCommands(pat.get(), matchedItems);

		if (matchedItems.IsEmpty()) {
			continue;
		}

		// 完全一致のものを探す
		command = nullptr;
		if (matchedItems.FindWholeMatchItem(&command) == false) {
			continue;
		}
		return command;
	}
	return nullptr;
}

bool CommandRepository::HasCommand(const CString& strQueryStr)
{
	CSingleLock sl(&in->mCS, TRUE);
	return  in->mCommands.Has(strQueryStr);
}

bool CommandRepository::IsValidAsName(const CString& strQueryStr)
{
	return strQueryStr.FindOneOf(_T(" !\"\\/*;:[]|&<>,")) == -1;
}

void CommandRepository::RegisterListener(CommandRepositoryListenerIF* listener)
{
	in->mListeners.push_back(listener);
}

void CommandRepository::UnregisterListener(CommandRepositoryListenerIF* listener)
{
	auto it = std::find(in->mListeners.begin(), in->mListeners.end(), listener);
	if (it != in->mListeners.end()) {
		in->mListeners.erase(it);
	}
}

void CommandRepository::OnAppFirstBoot()
{
	// 初回起動であることをコマンドプロバイダに通知
	for (auto& provider : in->mProviders) {
		provider->OnFirstBoot();
	}

	// コマンド設定ファイルを保存
	in->SaveCommands();

	// 問い合わせキュー処理用スレッドを起動する
	in->RunQueryThread();
}

void CommandRepository::OnAppNormalBoot()
{
	// 問い合わせキュー処理用スレッドを起動する
	in->RunQueryThread();
}

void CommandRepository::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	in->ReloadPatternObject();
}

void CommandRepository::OnAppExit()
{
	in->SetExit();

	in->mCommands.Clear();

	for (auto& provider : in->mProviders) {
		provider->Release();
	}
	in->mProviders.clear();

	AppPreference::Get()->UnregisterListener(this);
}

CString CommandRepository::IssueClonedCommandName(const CString& baseName)
{
	static tregex regex(_T("^.+-コピー$"));
	if (std::regex_match(tstring(baseName), regex) == false) {
		return baseName + _T("-コピー");
	}
	return baseName;
}



}
}

