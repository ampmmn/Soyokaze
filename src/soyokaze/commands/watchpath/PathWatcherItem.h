#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace watchpath {

class PathWatcherItem
{
public:
	bool BuildExcludeFilterRegex(std::unique_ptr<tregex>& reg) const;
	static void WildcardToRegexp(const CString& in, std::wstring& out);

	// 監視対象パス
	CString mPath;
	// 通知メッセージ
	CString mMessage;
	// 通知の間隔(前回の通知から次の通知までの間隔、秒数)
	UINT mInterval = 3600;
	// 除外パターン
	CString mExcludeFilter;
};

}
}
}

