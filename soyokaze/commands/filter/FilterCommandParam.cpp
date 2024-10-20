#include "pch.h"
#include "FilterCommandParam.h"

namespace launcherapp {
namespace commands {
namespace filter {

CommandParam::CommandParam() : 
	mAfterCommandParam(_T("$select")),
	mPreFilterType(0),
	mCacheType(0),
	mPostFilterType(0)
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
	mPostFilterType(rhs.mPostFilterType),
	mAfterCommandName(rhs.mAfterCommandName),
	mAfterFilePath(rhs.mAfterFilePath),
	mAfterCommandParam(rhs.mAfterCommandParam),
	mHotKeyAttr(rhs.mHotKeyAttr)
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
		mPostFilterType = rhs.mPostFilterType;
		mAfterCommandName = rhs.mAfterCommandName;
		mAfterFilePath = rhs.mAfterFilePath;
		mAfterCommandParam = rhs.mAfterCommandParam;
		mHotKeyAttr = rhs.mHotKeyAttr;
	}
	return *this;

}


} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

