#include "pch.h"
#include "IniWinScpSessionStorage.h"
#include "commands/winscp/WinScpCommandParam.h"
#include "commands/decodestring/DecodeUriCommand.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"
#include "utility/RegistryKey.h"
#include <fstream>
#include <ostream>
#include <vector>

namespace launcherapp { namespace commands { namespace winscp {

using DecodeUriCommand = launcherapp::commands::decodestring::DecodeUriCommand;

constexpr LPCTSTR INI_PATH_IN_APPDATA = _T("%APPDATA%\\WinSCP.ini");
constexpr LPCTSTR SUBKEY_CUSTIMINIPATH = _T("Software\\Martin Prikryl\\WinSCP 2 Override");

IniSessionStorage::IniSessionStorage()
{
}

IniSessionStorage::~IniSessionStorage()
{
}

static bool GetLastUpdateTime(LPCTSTR path, FILETIME& ftime)
{
	HANDLE h = CreateFile(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	GetFileTime(h, nullptr, nullptr, &ftime);
	CloseHandle(h);
	return true;
}

bool IniSessionStorage::HasUpdate()
{
	bool isFirstCall = false;
	if (mIniFilePath.IsEmpty()) {
		// 設定ファイルのパスを取得する
		if (FindIniFilePath(mIniFilePath) == false) {
			return false;
		}
		isFirstCall = true;
	}

	FILETIME ft;
	if (GetLastUpdateTime(mIniFilePath, ft) == false) {
		return false;
	}

	// 前回からファイルが更新されているかどうか
	bool hasUpdate = false;
	if (isFirstCall == false) {
		hasUpdate = memcmp(&mLastUpdateFt, &ft, sizeof(FILETIME)) == 0;
	}

	mLastUpdateFt = ft;

	return isFirstCall || hasUpdate;
}

bool IniSessionStorage::LoadSessions(std::vector<CString>& sessionNames)
{
	if (Path::FileExists(mIniFilePath) == false) {
		return false;
	}

	CString name;
	std::vector<CString> names;

	// 設定ファイルを読む
	// 行単位で読み込む。[Sessions\...]の...を取り出してURIデコードすればOK
	std::regex pattern("^\\[Sessions\\\\(.+)\\] *");
	std::ifstream file((LPCSTR)CStringA(mIniFilePath));
	std::string line;

	while (std::getline(file, line)) {
		if (std::regex_match(line, pattern) == false) {
			continue;
		}

		std::string session = std::regex_replace(line, pattern, "$1");
		// セッション名をURIデコードする
		DecodeUriCommand::DecodeURI(session, name);
		names.push_back(name);
	}

	sessionNames.swap(names);
	return true;
}

// INIファイルの設定情報が存在するか?
bool IniSessionStorage::Exists()
{
	// 設定ファイルの有無により判断する
	CString iniFilePath;
	return FindIniFilePath(iniFilePath);
}

bool IniSessionStorage::FindIniFilePath(CString& iniFilePath)
{
	// カスタムINIファイルのパスが設定されているか確認
	if (FindCustomIniFilePath(iniFilePath)) {
		return true;
	}
	return FindAutomaticIniFilePath(iniFilePath);
}


bool IniSessionStorage::FindAutomaticIniFilePath(CString& iniFilePath)
{
	// %APPDATA%直下にwinscp.iniがあるかどうかを確認
	DWORD sizeNeeded = ExpandEnvironmentStrings(INI_PATH_IN_APPDATA, nullptr, 0);
	std::vector<TCHAR> path(sizeNeeded);
	ExpandEnvironmentStrings(INI_PATH_IN_APPDATA, path.data(), sizeNeeded);

	if (Path::FileExists(path.data())) {
		iniFilePath = path.data();
		return true;
	}

	// WinSCP.exeと同じディレクトリにwinscp.iniがあるかどうかを確認
	auto pref = AppPreference::Get();
	CommandParam param;
	param.Load((Settings&)pref->GetSettings());

	// winscp.exeのパスを得る
	CString appExePath;
	if (param.ResolveExecutablePath(appExePath) == false || 
	    Path::FileExists(appExePath) == false) {
		return false;
	}

	// winscp.exe → winscp.ini
	iniFilePath = appExePath;
	PathRenameExtension(iniFilePath.GetBuffer(iniFilePath.GetLength()+1), _T(".ini"));
	iniFilePath.ReleaseBuffer();

	return Path::FileExists(iniFilePath);
}

bool IniSessionStorage::FindCustomIniFilePath(CString& iniFilePath)
{
	RegistryKey HKCU(HKEY_CURRENT_USER);

	CString value;

	RegistryKey keyIniPath;
	HKCU.OpenSubKey(SUBKEY_CUSTIMINIPATH, keyIniPath);
	if (keyIniPath.GetValue(_T("IniFile"), value) == false) {
		return false;
	}

	// BOMを除去
	value.Replace(_T("%EF%BB%BF"), _T(""));

	std::string tmp;
	UTF2UTF(value, tmp);
	DecodeUriCommand::DecodeURI(tmp, value);

	if (Path::FileExists(value) == false) {
		return false;
	}

	iniFilePath = value;
	return true;
}


}}} // end of namespace launcherapp::commands::winscp
