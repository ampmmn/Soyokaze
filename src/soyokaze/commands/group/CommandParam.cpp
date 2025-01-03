#include "pch.h"
#include "CommandParam.h"


namespace launcherapp {
namespace commands {
namespace group {

CommandParam::CommandParam() : 
	mIsPassParam(FALSE),
	mIsRepeat(FALSE),
	mRepeats(1),
	mIsConfirm(TRUE)
{
}

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mItems(rhs.mItems),
	mIsPassParam(rhs.mIsPassParam),
	mIsRepeat(rhs.mIsRepeat),
	mRepeats(rhs.mRepeats),
	mIsConfirm(rhs.mIsConfirm),
	mHotKeyAttr(rhs.mHotKeyAttr)
{

}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (
	const CommandParam& rhs
)
{
	if (&rhs != this) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mItems = rhs.mItems;
		mIsPassParam = rhs.mIsPassParam;
		mIsRepeat = rhs.mIsRepeat;
		mRepeats = rhs.mRepeats;
		mIsConfirm = rhs.mIsConfirm;
		mHotKeyAttr = rhs.mHotKeyAttr;
	}
	return *this;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp



