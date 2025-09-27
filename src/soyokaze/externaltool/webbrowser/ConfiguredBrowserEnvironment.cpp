#include "pch.h"
#include "ConfiguredBrowserEnvironment.h"
#include "externaltool/webbrowser/ChromeEnvironment.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/Path.h"
#include "utility/VersionInfo.h"

namespace launcherapp { namespace externaltool { namespace webbrowser {


struct ConfiguredBrowserEnvironment::PImpl : public AppPreferenceListenerIF
{
	PImpl() {
		// アプリ設定変更時に更新通知を受け取るための登録
		AppPreference::Get()->RegisterListener(this);
		// 設定項目をロード
		Load();
	}
	~PImpl() {
		AppPreference::Get()->UnregisterListener(this);
	}

	// アプリ設定から関連項目をロードする
	void Load() {
		auto pref = AppPreference::Get();
		auto& settings = pref->GetSettings();

		mIsEnable = settings.Get(_T("ExternalToolBrowser:EnableExternal"), true);
		mUseChrome = settings.Get(_T("ExternalToolBrowser:UseChrome"), true);
	
		mExeFilePath = settings.Get(_T("ExternalToolBrowser:ExeFilePath"), _T(""));
		mParameter = settings.Get(_T("ExternalToolBrowser:Parameter"), _T("$target"));
		mUserDataDirPath = settings.Get(_T("ExternalToolBrowser:DataDirPath"), _T(""));
		mDisplayName = settings.Get(_T("ExternalToolBrowser:DisplayName"), _T(""));
		mShouleOpenUrlWithSystem = settings.Get(_T("ExternalToolBrowser:OpenUrlWithSystem"), true);
	}
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}

	// アプリ設定の更新通知
	void OnAppPreferenceUpdated() override
 	{
		Load();
	}

	void OnAppExit() override {}

	// Webブラウザ(外部ツール)の表示名
	CString mDisplayName;
	// Webブラウザ(外部ツール)の実行ファイルパス
	CString mExeFilePath;
	// Webブラウザ(外部ツール)のパラメータ
	CString mParameter;
	// Webブラウザ(外部ツール)の設定フォルダパス
	CString mUserDataDirPath;

	// Webブラウザ(外部ツール)を利用するか?
	bool mIsEnable{true};
	// 外部ツールはChromeか?
	bool mUseChrome{true};
	// URLを開くときに既定のブラウザを使用するか?
	bool mShouleOpenUrlWithSystem{true};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



ConfiguredBrowserEnvironment::ConfiguredBrowserEnvironment() : in(new PImpl)
{
}

ConfiguredBrowserEnvironment::~ConfiguredBrowserEnvironment()
{
}


ConfiguredBrowserEnvironment* ConfiguredBrowserEnvironment::GetInstance()
{
	static ConfiguredBrowserEnvironment inst;
	return &inst;
}

void ConfiguredBrowserEnvironment::Load()
{
	in->Load();
}

bool ConfiguredBrowserEnvironment::ShouldUseThisFor(const CString& url)
{
	UNREFERENCED_PARAMETER(url);
	
	return in->mShouleOpenUrlWithSystem == false;
}

bool ConfiguredBrowserEnvironment::IsAvailable()
{
	CString dummyPath;
	return GetInstalledExePath(dummyPath);
}

// 実行パラメータを取得する
bool ConfiguredBrowserEnvironment::GetCommandlineParameter(CString& param)
{
	if (in->mIsEnable == false) {
		return false;
	}

	if (in->mUseChrome) {
		return ChromeEnvironment::GetInstance()->GetCommandlineParameter(param);
	}
	param = in->mParameter;
	return true;
}

bool ConfiguredBrowserEnvironment::GetInstalledExePath(CString& path)
{
	if (in->mIsEnable == false) {
		return false;
	}

	if (in->mUseChrome) {
		return ChromeEnvironment::GetInstance()->GetInstalledExePath(path);
	}

	path = in->mExeFilePath;
	return Path::FileExists(in->mExeFilePath);
}

// ブックマークデータのパスを取得
bool ConfiguredBrowserEnvironment::GetBookmarkFilePath(CString& path)
{
	if (in->mIsEnable == false) {
		return false;
	}

	if (in->mUseChrome) {
		return ChromeEnvironment::GetInstance()->GetBookmarkFilePath(path);
	}

	if (Path::IsDirectory(in->mUserDataDirPath) == false) {
		return false;
	}

	Path pathBkm(in->mUserDataDirPath);
	pathBkm.Append(_T("Default\\Bookmarks"));
	if (pathBkm.FileExists()) {
		path = (LPCTSTR)pathBkm;
		return true;
	}
	pathBkm = in->mUserDataDirPath;
	pathBkm.Append(_T("Bookmarks"));

	if (pathBkm.FileExists()) {
		path = (LPCTSTR)pathBkm;
		return true;
	}
	return false;
}

// 履歴ファイルのパスを取得
bool ConfiguredBrowserEnvironment::GetHistoryFilePath(CString& path)
{
	if (in->mIsEnable == false) {
		return false;
	}

	if (in->mUseChrome) {
		return ChromeEnvironment::GetInstance()->GetHistoryFilePath(path);
	}

	if (Path::IsDirectory(in->mUserDataDirPath) == false) {
		// 設定フォルダが存在しない
		return false;
	}

	// 設定フォルダがDefaultディレクトリの親ディレクトリをさしている場合
	Path pathHistory(in->mUserDataDirPath);
	pathHistory.Append(_T("Default\\History"));
	if (pathHistory.FileExists()) {
		path = (LPCTSTR)pathHistory;
		return true;
	}

	// 設定フォルダがDefaultディレクトリをさしている場合
	pathHistory = in->mUserDataDirPath;
	pathHistory.Append(_T("History"));

	if (pathHistory.FileExists()) {
		path = (LPCTSTR)pathHistory;
		return true;
	}
	return false;
}

// 製品名を取得
bool ConfiguredBrowserEnvironment::GetProductName(CString& name)
{
	if (in->mIsEnable == false) {
		return false;
	}
	if (in->mUseChrome) {
		return ChromeEnvironment::GetInstance()->GetProductName(name);
	}
	if (in->mDisplayName.IsEmpty() == FALSE) {
		name = in->mDisplayName;
		return true;
	}

	CString path;
	if (GetInstalledExePath(path) == false) {
		return false;
	}
	return VersionInfo::GetProductName(path, name);
}

}}}

