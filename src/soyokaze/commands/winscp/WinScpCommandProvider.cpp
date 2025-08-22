#include "pch.h"
#include "WinScpCommandProvider.h"
#include "commands/winscp/WinScpCommandParam.h"
#include "commands/winscp/WinScpCommand.h"
#include "commands/winscp/RegistryWinScpSessionStorage.h"
#include "commands/winscp/IniWinScpSessionStorage.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include <vector>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace winscp {



struct WinScpCommandProvider::PImpl : 
	public AppPreferenceListenerIF,
	public LauncherWindowEventListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		LauncherWindowEventDispatcher::Get()->AddListener(this);
	}
	virtual ~PImpl()
	{
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);
		AppPreference::Get()->UnregisterListener(this);
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {}

// LauncherWindowEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {
		if (mParam.mIsEnable == false) {
			return ;
		}
		if (mSessionStorage.get() == nullptr) {
			return ;
		}

		if (mSessionStorage->HasUpdate() == false) {
			// 変更なし
			return ;
		}

		// 再読み込み
		LoadSessions();
	}

	void OnLauncherActivate() override
	{
	}
	void OnLauncherUnactivate() override
	{
	}


	void Load();
	void LoadSessions();

	CommandParam mParam;
	std::vector<CString> mSessionNames;
	std::unique_ptr<SessionStrage> mSessionStorage;
	std::mutex mMutex;
};

/**
	レジストリを参照し、保存されたセッション名の一覧を取得する
*/
void WinScpCommandProvider::PImpl::Load()
{
	auto pref = AppPreference::Get();
	mParam.Load((Settings&)pref->GetSettings());

	if (IniSessionStorage::Exists()) {
		// INIファイル(自動 or カスタム)の設定情報があったらそれを使用する
		mSessionStorage.reset(new IniSessionStorage());
	}
	else {
		// なければ、レジストリの設定情報を使用する
		mSessionStorage.reset(new RegistrySessionStorage());
	}

	LoadSessions();
}

void WinScpCommandProvider::PImpl::LoadSessions()
{
	std::lock_guard<std::mutex> lock(mMutex); 
	mSessionStorage->LoadSessions(mSessionNames);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WinScpCommandProvider)


WinScpCommandProvider::WinScpCommandProvider() : in(std::make_unique<PImpl>())
{
}

WinScpCommandProvider::~WinScpCommandProvider()
{
}

CString WinScpCommandProvider::GetName()
{
	return _T("WinScp");
}

// 一時的なコマンドの準備を行うための初期化
void WinScpCommandProvider::PrepareAdhocCommands()
{
	// セッション名の一覧を取得
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void WinScpCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mParam.mIsEnable == false) {
		return;
	}

	std::lock_guard<std::mutex> lock(in->mMutex); 
	for (auto& sessionName : in->mSessionNames) {

		// セッション名と入力ワードがマッチするかを判定
		int level = pattern->Match(sessionName);
		if (level == Pattern::Mismatch) {
			continue;
		}
		commands.Add(CommandQueryItem(level, new WinScpCommand(&in->mParam, sessionName)));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t WinScpCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(WinScpCommand::TypeDisplayName());
	return 1;
}

}}} // end of namespace launcherapp::commands::winscp

