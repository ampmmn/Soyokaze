#include "pch.h"
#include "RegistryWinScpSessionStorage.h"
#include "utility/RegistryKey.h"
#include "commands/decodestring/DecodeUriCommand.h"
#include <vector>

namespace launcherapp { namespace commands { namespace winscp {

using DecodeUriCommand = launcherapp::commands::decodestring::DecodeUriCommand;

constexpr LPCTSTR SUBKEY_SESSIONS = _T("Software\\Martin Prikryl\\WinSCP 2\\Sessions");


RegistrySessionStorage::RegistrySessionStorage()
{
}

RegistrySessionStorage::~RegistrySessionStorage()  
{
	if (mEventForNotify) {
		CloseHandle(mEventForNotify);
	}
	if (mSubKeyForWatch) {
		RegCloseKey(mSubKeyForWatch);
	}
}

bool RegistrySessionStorage::HasUpdate()
{
	if (OpenSubKeyIfNotOpened() == false) {
		return false;
	}

	// レジストリに変更があったかどうかをチェック
	if (WaitForSingleObject(mEventForNotify, 0) == WAIT_TIMEOUT) {
		// 変更なし
		return false;
	}

	// 次の通知を受け取れるようにするため、再登録する
	RegisterNotifyChangeKeyValue();
	return true;
}

bool RegistrySessionStorage::LoadSessions(std::vector<CString>& sessionNames)
{
	RegistryKey HKCU(HKEY_CURRENT_USER);

	RegistryKey keySessions;
	HKCU.OpenSubKey(SUBKEY_SESSIONS, keySessions);

	std::vector<CString> names;
	keySessions.EnumSubKeyNames(names);

	// URIデコードする
	std::string tmp;
	for (auto& name : names) {
		UTF2UTF(name, tmp);
		DecodeUriCommand::DecodeURI(tmp, name);
	}

	sessionNames.swap(names);
	return true;
}

bool RegistrySessionStorage::OpenSubKeyIfNotOpened()
{
	if (mSubKeyForWatch) {
		return true;
	}

	// レジストリに変更があったかどうかをみる
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

// レジストリの変更があったら通知を受け取るための登録をする
bool RegistrySessionStorage::RegisterNotifyChangeKeyValue()
{
	HKEY hKey = mSubKeyForWatch;
	HANDLE hEvent = mEventForNotify;

	// イベント状態を非シグナル状態にもどす
	ResetEvent(hEvent);

	// 通知を登録(変更があったらイベントがシグナル状態になる)
	LONG result = RegNotifyChangeKeyValue(hKey, TRUE, REG_NOTIFY_CHANGE_NAME, hEvent, TRUE);
	if (result != ERROR_SUCCESS) {
		spdlog::error(_T("Failed to notify {}"), result);
		return false;
	}
	return true;
}

}}} // end of namespace launcherapp::commands::winscp
