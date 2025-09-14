#pragma once

#include "commands/onenote/OneNoteSection.h"
#include <vector>

namespace launcherapp { namespace commands { namespace onenote {

class OneNoteBook
{
public:

	// ページ名を取得
	LPCWSTR GetName() const { return mName; }
	LPCWSTR GetNickName() const { return mNickName; }
	LPCWSTR GetID() const { return mID; }

	CStringW mID;
	CStringW mName;
	CStringW mNickName;
	CTime mLastModified;
	std::vector<OneNoteSection> mSections;
};

}}} // end of namespace launcherapp::commands::onenote
