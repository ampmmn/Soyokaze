#include "pch.h"
#include "ShellExecCommandParam.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace shellexecute {


CommandParam::CommandParam() :
	mIsRunAsAdmin(FALSE),
	mShowType(0),
	mIsUse0(FALSE),
	mIsShowArgDialog(FALSE),
	mIsUseDescriptionForMatching(FALSE)
{
}


CommandParam::CommandParam(const CommandParam& rhs)
{
	mName = rhs.mName;
	mDescription = rhs.mDescription;
	mIsRunAsAdmin = rhs.mIsRunAsAdmin;
	mShowType = rhs.mShowType;
	mDir = rhs.mDir;
	mPath = rhs.mPath;
	mParameter = rhs.mParameter;
	mPath0 = rhs.mPath0;
	mParameter0 = rhs.mParameter0;
	mIsUse0 = rhs.mIsUse0;
	mIsShowArgDialog = rhs.mIsShowArgDialog;
	mHotKeyAttr = rhs.mHotKeyAttr;
	mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
	mIconData = rhs.mIconData;
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (&rhs != this) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mIsRunAsAdmin = rhs.mIsRunAsAdmin;
		mShowType = rhs.mShowType;
		mDir = rhs.mDir;
		mPath = rhs.mPath;
		mParameter = rhs.mParameter;
		mPath0 = rhs.mPath0;
		mParameter0 = rhs.mParameter0;
		mIsUse0 = rhs.mIsUse0;
		mIsShowArgDialog = rhs.mIsShowArgDialog;
		mHotKeyAttr = rhs.mHotKeyAttr;
		mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
		mIconData = rhs.mIconData;
	}
	return *this;
}


int CommandParam::GetShowType() const
{
	if (mShowType == 1) {
		return SW_MAXIMIZE;
	}
	else if (mShowType == 2) {
		return SW_SHOWMINIMIZED;
	}
	else {
		return SW_NORMAL;
	}
}

void CommandParam::SetShowType(int type)
{
	if (type == SW_SHOWMINIMIZED) {
		mShowType = 2;
	}
	else if (type == SW_MAXIMIZE) {
		mShowType = 1;
	}
	else {
		mShowType = 0;
	}
}

}
}
}

