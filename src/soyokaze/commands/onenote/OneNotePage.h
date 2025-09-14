#pragma once

namespace launcherapp { namespace commands { namespace onenote {

class OneNotePage
{
public:

	// ページ名を取得
	LPCWSTR GetName() const { return mName; }
	LPCWSTR GetID() const { return mID; }

	CStringW mID;
	CStringW mName;
	CTime mDateTime;
	CTime mLastModifiedTime;
	bool mIsSubPage{false};
};

}}} // end of namespace launcherapp::commands::onenote
