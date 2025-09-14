#pragma once

#include "commands/onenote/OneNotePage.h"
#include <vector>

namespace launcherapp { namespace commands { namespace onenote {

class OneNoteSection
{
public:
	LPCWSTR GetName() const { return mName; }
	LPCWSTR GetID() const { return mID; }
	const std::vector<OneNotePage>& GetPages() const { return mPages; }

	CStringW mID;
	CStringW mName;
	CTime mLastModifiedTime;
	std::vector<OneNotePage> mPages;
};


}}} // end of namespace launcherapp::commands::onenote
