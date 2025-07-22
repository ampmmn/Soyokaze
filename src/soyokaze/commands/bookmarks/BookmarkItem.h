#pragma once

namespace launcherapp { namespace commands { namespace bookmarks {

enum BrowserType {
	Chrome,
	Edge,
};

struct Bookmark
{
	// ブラウザ種別からブラウザ名を得る
	CString GetBrowserName();
	// ブラウザ種別に対応するブラウザexeのパスを得る
	bool GetExecutablePath(LPTSTR path, size_t len);

	// 表示名
	CString mName;
	// フォルダ階層
	CString mFolderPath;
	// URL
	CString mUrl;
	// ブラウザ種別
	int mBrowser{0};
	// 一致レベル(これは別にすべきでは?
	int mMatchLevel{0};
};

}}} // end of namespace launcherapp::commands::bookmarks

