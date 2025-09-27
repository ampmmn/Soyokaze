#pragma once

namespace launcherapp { namespace commands { namespace bookmarks {

struct Bookmark
{
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

