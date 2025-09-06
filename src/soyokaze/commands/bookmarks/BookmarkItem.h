#pragma once

namespace launcherapp { namespace commands { namespace bookmarks {

enum BrowserType {
	Chrome,
	Edge,
	Unknown,
};

struct Bookmark
{
	// ブラウザ種別に対応するブラウザexeのパスを得る
	static bool GetExecutablePath(BrowserType type, LPTSTR path, size_t len);
	// ブラウザ種別からブラウザ名を得る
	static CString GetBrowserName(BrowserType type);

	// 表示名
	CString mName;
	// フォルダ階層
	CString mFolderPath;
	// URL
	CString mUrl;
	// 一致レベル(これは別にすべきでは?
	int mMatchLevel{0};
};

}}} // end of namespace launcherapp::commands::bookmarks

