#include "pch.h"
#include "WinScpCommandProvider.h"
#include "commands/winscp/WinScpCommandParam.h"
#include "commands/winscp/WinScpCommand.h"
#include "commands/decodestring/DecodeUriCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "utility/RegistryKey.h"
#include <vector>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace winscp {

using DecodeUriCommand = launcherapp::commands::decodestring::DecodeUriCommand;

constexpr LPCWSTR SUBKEY_SESSIONS = L"Software\\Martin Prikryl\\WinSCP 2\\Sessions";


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

		if (mEventForNotify) {
			CloseHandle(mEventForNotify);
		}
		if (mSubKeyForWatch) {
			RegCloseKey(mSubKeyForWatch);
		}
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Reload();
	}
	void OnAppExit() override {}

// LauncherWindowEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {
		if (mParam.mIsEnable == false) {
			return ;
		}

		if (OpenSubKeyIfNotOpened() == false) {
			return;
		}

		if (WaitForSingleObject(mEventForNotify, 0) == WAIT_TIMEOUT) {
			return ;
		}

		RegisterNotifyChangeKeyValue();
		ReloadSessions();
	}

	void OnLauncherActivate() override
	{
	}
	void OnLauncherUnactivate() override
	{
	}


	void Reload();
	void ReloadSessions();

	bool OpenSubKeyIfNotOpened() {

		if (mSubKeyForWatch) {
			return true;
		}

		HKEY hKey{nullptr};
		LONG result = RegOpenKeyEx(HKEY_CURRENT_USER, SUBKEY_SESSIONS, 0, KEY_READ, &hKey);
		if (result != ERROR_SUCCESS) {
			spdlog::error(_T("Failed to open registry key {0} result:{1}"), SUBKEY_SESSIONS, result);
			return false;
		}

		HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

		mSubKeyForWatch = hKey;
		mEventForNotify = hEvent;

		return RegisterNotifyChangeKeyValue();

	}

	bool RegisterNotifyChangeKeyValue()
	{
		HKEY hKey = mSubKeyForWatch;
		HANDLE hEvent = mEventForNotify;

		ResetEvent(hEvent);
		LONG result = RegNotifyChangeKeyValue(hKey, TRUE, REG_NOTIFY_CHANGE_NAME, hEvent, TRUE);
		if (result != ERROR_SUCCESS) {
			spdlog::error(_T("Failed to notify {}"), result);
			return false;
    }
		return true;
	}

	CommandParam mParam;
	std::vector<CString> mSessionNames;
	HKEY mSubKeyForWatch{nullptr};
	HANDLE mEventForNotify{nullptr};
	std::mutex mMutex;
};

/**
	レジストリを参照し、保存されたセッション名の一覧を取得する
*/
void WinScpCommandProvider::PImpl::Reload()
{
	auto pref = AppPreference::Get();
	mParam.Load((Settings&)pref->GetSettings());

	ReloadSessions();
}

void WinScpCommandProvider::PImpl::ReloadSessions()
{
	std::vector<CString> sessionNames;

	RegistryKey keySessions;

	RegistryKey HKCU(HKEY_CURRENT_USER);
	HKCU.OpenSubKey(SUBKEY_SESSIONS, keySessions);

	keySessions.EnumSubKeyNames(sessionNames);

	// URIデコードする
	std::string tmp;
	for (auto& sessionName : sessionNames) {
		UTF2UTF(sessionName, tmp);
		DecodeUriCommand::DecodeURI(tmp, sessionName);
	}

	std::lock_guard<std::mutex> lock(mMutex); 
	mSessionNames.swap(sessionNames);
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
	in->Reload();
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

