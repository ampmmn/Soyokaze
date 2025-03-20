// あ
#include "pch.h"
#include "WatchPathCommandParam.h"
#include "commands/core/CommandEntryIF.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace watchpath {

CommandParam::CommandParam() :
 	mNotifyMessage(_T("更新を検知")), 
	mWatchInterval(300),
	mIsDisabled(false)
{
}

CommandParam::~CommandParam()
{
}

bool CommandParam::operator == (const CommandParam& rhs) const
{
	if (&rhs == this) { return true; }

	return mName == rhs.mName &&
	       mDescription == rhs.mDescription &&
	       mPath == rhs.mPath &&
	       mNotifyMessage == rhs.mNotifyMessage &&
	       mWatchInterval == rhs.mWatchInterval &&
				 mExcludeFilter == rhs.mExcludeFilter &&
	       mIsDisabled == rhs.mIsDisabled;
}


bool CommandParam::Save(CommandEntryIF* entry)
{
	ASSERT(entry);
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("path"), mPath);
	entry->Set(_T("message"), mNotifyMessage);
	entry->Set(_T("watchinterval"), (int)mWatchInterval);
	entry->Set(_T("excludefilter"), mExcludeFilter);
	entry->Set(_T("isDisabled"), mIsDisabled);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mPath = entry->Get(_T("path"), _T(""));
	mNotifyMessage = entry->Get(_T("message"), _T(""));
	mWatchInterval = (UINT)entry->Get(_T("watchinterval"), 300);
	mExcludeFilter = entry->Get(_T("excludefilter"), _T(""));
	mIsDisabled = entry->Get(_T("isDisabled"), false);

	return true;
}


void CommandParam::swap(CommandParam& rhs)
{
	std::swap(mName, rhs.mName);
	std::swap(mDescription, rhs.mDescription);
	std::swap(mPath, rhs.mPath);
	std::swap(mNotifyMessage, rhs.mNotifyMessage);
	std::swap(mWatchInterval, rhs.mWatchInterval);
	std::swap(mExcludeFilter, rhs.mExcludeFilter);
	std::swap(mIsDisabled, rhs.mIsDisabled);
}

}
}
}

