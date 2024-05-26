#include "pch.h"
#include "CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "commands/core/CommandMap.h"
#include "commands/core/CommandQueryRequest.h"
#include "utility/AppProfile.h"
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

using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;
using CommandRanking = launcherapp::commands::core::CommandRanking;

namespace launcherapp {
namespace core {

using QueryRequest = launcherapp::commands::core::CommandQueryRequest;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct CommandRepository::PImpl
{
	void ReloadPatternObject()
	{
		SPDLOG_DEBUG(_T("start"));

		mPattern.reset(new PartialMatchPattern());

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
			HOTKEY_ATTR attr;
			hotKeyMap.GetHotKeyAttr(i, attr);

			auto handler = std::make_unique<NamedCommandHotKeyHandler>(name);
			bool isGlobal = hotKeyMap.IsGlobal(i);

			hotKeyManager->Register(this, handler.release(), attr, isGlobal);
		}
		SPDLOG_DEBUG(_T("end"));
	}

	void SaveCommands()
	{
		SPDLOG_DEBUG(_T("start"));

		CommandFile commandFile;
		commandFile.SetFilePath(mCommandFilePath);

		std::vector<launcherapp::core::Command*> commands;
		for (auto& command : mCommands.Enumerate(commands)) {
			command->Save(&commandFile);
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

	CCriticalSection mCS;

	std::vector<CommandProvider*> mProviders;

	// リスナー
	std::vector<CommandRepositoryListenerIF*> mListeners;

	// 設定ファイル(commands.ini)のパス
	CString mCommandFilePath;

	// 一般コマンド一覧
	CommandMap mCommands;
	// キーワード比較用のクラス
	std::unique_ptr<Pattern> mPattern;
	
	// 編集中フラグ
	bool mIsNewDialog = false;
	bool mIsEditDialog = false;
	bool mIsManagerDialog = false;
	bool mIsRegisteFromFileDialog = false;

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

			while (IsRunning()) {
				QueryRequest req;
				if (WaitForRequest(req) == false) {
					continue;
				}
				Query(req);
			}

	});
	th.detach();

	mIsThreadInitialized = true;
}

void CommandRepository::PImpl::AddRequest(const QueryRequest& req)
{
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mQueryRequestQueue.push_back(req);
	}
	mQueryRequestEvt.SetEvent();
}

bool CommandRepository::PImpl::WaitForRequest(QueryRequest& req)
{
	WaitForSingleObject(mQueryRequestEvt, INFINITE);

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
	const CommandParameter& param = req.GetCommandParameter();
	HWND hwndNotify = req.GetNotifyWindow();
	UINT notifyMsg = req.GetNotifyMessage();

	// パラメータが空の場合は検索しない
	if (param.IsEmpty()) {
		// 結果なし
		PostMessage(hwndNotify, notifyMsg, 0, 0);
		return;
	}

	CSingleLock sl(&mCS, TRUE);

	spdlog::stopwatch swAll;

	// 入力文字列をを設定
	mPattern->SetParam(param);

	CommandMap::CommandQueryItemList matchedItems;

	spdlog::stopwatch sw;
	mCommands.Query(mPattern.get(), matchedItems);
	spdlog::debug("CommandMap.Query duration:{:.6f} s.", sw);
	  // Note: ここで+1した参照カウントは CommandRepository::Query 呼び出し元で-1する必要あり

	// コマンドプロバイダーから一時的なコマンドを取得する
	for (auto& provider : mProviders) {
		sw.reset();
		if (HasSubsequentRequest()) {
			// キャンセル通知
			PostMessage(hwndNotify, notifyMsg, 1, 0);
			break;
		}
		provider->QueryAdhocCommands(mPattern.get(), matchedItems);
		spdlog::debug(_T("QueryAdhocCommands name:{0} duration:{1:.6f} s."), (LPCTSTR)provider->GetName(), sw.elapsed().count());
	}

	// 一致レベルに基づきソート
	const CommandRanking* rankPtr = CommandRanking::GetInstance();

	sw.reset();

	std::stable_sort(matchedItems.begin(), matchedItems.end(),
		[rankPtr](const CommandMap::CommandQueryItem& l, const CommandMap::CommandQueryItem& r) {
			if (r.mMatchLevel < l.mMatchLevel) { return true; }
			if (r.mMatchLevel > l.mMatchLevel) { return false; }

			// 一致レベルが同じ場合は優先順位による判断を行う
			auto& cmdL = l.mCommand;
			auto& cmdR = r.mCommand;

			auto nameL = cmdL->GetName();
			auto nameR = cmdR->GetName();

			int priorityL = cmdL->IsPriorityRankEnabled() ? rankPtr->Get(nameL) : 0;
			int priorityR = cmdR->IsPriorityRankEnabled() ? rankPtr->Get(nameR) : 0;
			return priorityR < priorityL;
	});

	spdlog::debug("sort items:{0} duration:{1:.6f} s.", (int)matchedItems.size(), sw);

	std::vector<launcherapp::core::Command*>* items = new std::vector<launcherapp::core::Command*>();
	items->reserve(matchedItems.size());
	for (auto& item : matchedItems) {
		item.mCommand->AddRef();
		items->push_back(item.mCommand.get());
	}

	spdlog::debug("Query took about {:.6f} seconds.", swAll);

	PostMessage(hwndNotify, notifyMsg, 0, (LPARAM)items);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandRepository::CommandRepository() : in(std::make_unique<PImpl>())
{
	AppPreference::Get()->RegisterListener(this);

	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetDirPath(path, MAX_PATH_NTFS);
	PathAppend(path, _T("commands.ini"));
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
int CommandRepository::RegisterCommand(Command* command)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Register(command);
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

	CommandRanking::GetInstance()->Delete(command->GetName());
	in->mCommands.Unregister(command);
	return 0;
}

// 名前変更による登録しなおし
int CommandRepository::ReregisterCommand(Command* command)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Reregister(command);
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
int CommandRepository::NewCommandDialog(const CommandParameter* param)
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
	if (param == nullptr || param->GetNamedParam(_T("TYPE"), &typeStr) == false) {
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
int CommandRepository::EditCommandDialog(const CString& cmdName)
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

	int ret =cmdAbs->EditDialog();
	cmdAbs->Release();

	if (ret != 0) {
		return 2;
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
	if (in->mIsManagerDialog) {
		// 編集操作中の再入はしない
		SPDLOG_WARN(_T("re-entry is not allowed."));
		return 0;
	}
	ScopeEdit scopeEdit(in->mIsManagerDialog);

	// キャンセル時用のバックアップ
	CommandMap commandsBkup(in->mCommands);

	KeywordManagerDialog dlg;
	if (dlg.DoModal() != IDOK) {

		// OKではないので結果を反映しない(バックアップした内容に戻す)
		in->mCommands.Swap(commandsBkup);
	}
	else {
		// ファイルに保存
	in->SaveCommands();
	}
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

			CString name(PathFindFileName(filePath));
			PathRemoveExtension(name.GetBuffer(name.GetLength()));
			name.ReleaseBuffer();
			
			if (name.IsEmpty()) {
				// .xxx というファイル名の場合にnameが空文字になるのを回避する
				name = PathFindFileName(filePath);
			}

			// パスとして使えるが、ShellExecCommandのコマンド名として許可しない文字をカットする
			ShellExecCommand::SanitizeName(name);

			CString suffix;
			
			for (int i = 1;; ++i) {
				auto cmd = QueryAsWholeMatch(name + suffix, false);
				if (cmd == nullptr) {
					break;
				}
				cmd->Release();
				suffix.Format(_T("(%d)"), i);
			}

			if (suffix.IsEmpty() == FALSE) {
				name = name + suffix;
			}

			// ダイアログで入力された内容に基づき、コマンドを新規作成する
			auto newCmd = std::make_unique<ShellExecCommand>();
			newCmd->SetName(name);

			ShellExecCommand::ATTRIBUTE normalAttr;
			normalAttr.mPath = filePath;
			newCmd->SetAttribute(normalAttr);
			in->mCommands.Register(newCmd.release());
		}

		AfxMessageBox(_T("登録しました"));
	}
	else {
		// 登録ダイアログを表示

		CString filePath = files[0];

		CString name(PathFindFileName(filePath));
		PathRemoveExtension(name.GetBuffer(name.GetLength()));
		name.ReleaseBuffer();

		CommandParameter param;
		param.SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param.SetNamedParamString(_T("COMMAND"), name);
		param.SetNamedParamString(_T("PATH"), filePath);

		return NewCommandDialog(&param);
	}
	return 0;
}

void CommandRepository::EnumCommands(std::vector<launcherapp::core::Command*>& enumCommands)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Enumerate(enumCommands);
}

void
CommandRepository::Query(
	const QueryRequest& newRequest
)
{
	// 初回に問い合わせスレッドを起動する
	in->RunQueryThread();
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

	WholeMatchPattern pat(strQueryStr);

	auto command = in->mCommands.FindOne(&pat);
	if (command != nullptr) {
		return command;
	}

	if (isIncludeAdhocCommand == false) {
		return nullptr;
	}

	// コマンドプロバイダーから一時的なコマンドを取得する
	launcherapp::CommandQueryItemList matchedItems;
	for (auto& provider : in->mProviders) {
		provider->QueryAdhocCommands(&pat, matchedItems);

		if (matchedItems.empty()) {
			continue;
		}

		// 完全一致のものを探す
		for (auto& item : matchedItems) {
			if (item.mMatchLevel != Pattern::WholeMatch) {
				continue;
			}
			command = item.mCommand.get();
			command->AddRef();

			return command;
		}
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

}

void CommandRepository::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	in->ReloadPatternObject();
}

void CommandRepository::OnAppExit()
{
	in->SetExit();

	// 優先度情報をファイルに保存する
	CommandRanking::GetInstance()->Save();

	for (auto& provider : in->mProviders) {
		provider->Release();
	}
	in->mProviders.clear();

	AppPreference::Get()->UnregisterListener(this);
}

}
}

