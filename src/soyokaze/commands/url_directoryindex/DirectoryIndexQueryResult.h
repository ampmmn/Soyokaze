#pragma once

#include <vector>
#include "matcher/Pattern.h"

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

struct QueryResult
{
	CString mLinkPath;
	CString mLinkText;
	CString mURL;
	int mMatchLevel = Pattern::Mismatch;
};

using DirectoryIndexQueryResult = std::vector<QueryResult>;


} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

