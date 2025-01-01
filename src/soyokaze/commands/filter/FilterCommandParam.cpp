#include "pch.h"
#include "FilterCommandParam.h"

namespace launcherapp {
namespace commands {
namespace filter {

CommandParam::CommandParam() : 
	mAfterCommandParam(_T("$select")),
	mPreFilterType(0),
	mCacheType(0),
	mPostFilterType(0),
	mPreFilterCodePage(CP_UTF8)
{
}

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mDir(rhs.mDir),
	mPath(rhs.mPath),
	mParameter(rhs.mParameter),
	mPreFilterType(rhs.mPreFilterType),
	mCacheType(rhs.mCacheType),
	mPreFilterCodePage(rhs.mPreFilterCodePage),
	mPostFilterType(rhs.mPostFilterType),
	mAfterCommandName(rhs.mAfterCommandName),
	mAfterFilePath(rhs.mAfterFilePath),
	mAfterCommandParam(rhs.mAfterCommandParam),
	mHotKeyAttr(rhs.mHotKeyAttr),
	mAfterDir(rhs.mAfterDir),
	mAfterShowType(rhs.mAfterShowType)
{
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (this != &rhs) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mDir = rhs.mDir;
		mPath = rhs.mPath;
		mParameter = rhs.mParameter;
		mPreFilterType = rhs.mPreFilterType;
		mCacheType = rhs.mCacheType;
		mPreFilterCodePage = rhs.mPreFilterCodePage;
		mPostFilterType = rhs.mPostFilterType;
		mAfterCommandName = rhs.mAfterCommandName;
		mAfterFilePath = rhs.mAfterFilePath;
		mAfterCommandParam = rhs.mAfterCommandParam;
		mHotKeyAttr = rhs.mHotKeyAttr;
		mAfterDir = rhs.mAfterDir;
		mAfterShowType = rhs.mAfterShowType;
	}
	return *this;

}

int CommandParam::GetAfterShowType() const
{
	return mAfterShowType;
}

bool CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);

	entry->Set(_T("path"), mPath);
	entry->Set(_T("dir"), mDir);
	entry->Set(_T("parameter"), mParameter);
	entry->Set(_T("prefiltertype"), mPreFilterType);
	entry->Set(_T("cachetype"), mCacheType);
	entry->Set(_T("aftertype"), mPostFilterType);
	entry->Set(_T("aftercommand"), mAfterCommandName);
	entry->Set(_T("afterfilepath"), mAfterFilePath);
	entry->Set(_T("afterparam"), mAfterCommandParam);
	entry->Set(_T("afterdir"), mAfterDir);
	entry->Set(_T("aftershowtype"), mAfterShowType);
	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mPath = entry->Get(_T("path"), _T(""));
	mDir = entry->Get(_T("dir"), _T(""));
	mParameter = entry->Get(_T("parameter"), _T(""));
	mPreFilterType = entry->Get(_T("prefiltertype"), 0);
	mCacheType = entry->Get(_T("cachetype"), 0);
	mPostFilterType = entry->Get(_T("aftertype"), 0);
	mAfterCommandName = entry->Get(_T("aftercommand"), _T(""));
	mAfterFilePath = entry->Get(_T("afterfilepath"), _T(""));
	mAfterCommandParam = entry->Get(_T("afterparam"), _T("$select"));
	mAfterDir = entry->Get(_T("afterdir"), _T(""));
	mAfterShowType = entry->Get(_T("aftershowtype"), 0);
	return true;
}


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

