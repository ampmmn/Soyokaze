#include "pch.h"
#include "FilterCommandParam.h"

namespace soyokaze {
namespace commands {
namespace filter {

CommandParam::CommandParam() : mShowType(SW_NORMAL),
	mAfterCommandParam(_T("$select")),
	mPreFilterType(0),
	mPostFilterType(0)
{
}

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mShowType(rhs.mShowType),
	mDir(rhs.mDir),
	mPath(rhs.mPath),
	mParameter(rhs.mParameter),
	mPreFilterType(rhs.mPreFilterType),
	mPostFilterType(rhs.mPostFilterType),
	mAfterCommandName(rhs.mAfterCommandName),
	mAfterFilePath(rhs.mAfterFilePath),
	mAfterCommandParam(rhs.mAfterCommandParam)
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
		mShowType = rhs.mShowType;
		mDir = rhs.mDir;
		mPath = rhs.mPath;
		mParameter = rhs.mParameter;
		mPreFilterType = rhs.mPreFilterType;
		mPostFilterType = rhs.mPostFilterType;
		mAfterCommandName = rhs.mAfterCommandName;
		mAfterFilePath = rhs.mAfterFilePath;
		mAfterCommandParam = rhs.mAfterCommandParam;
	}
	return *this;

}


} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

