#include "pch.h"
#include "SimpleDictParam.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp {
namespace commands {
namespace simple_dict {

SimpleDictParam::SimpleDictParam() :
	mIsFirstRowHeader(TRUE),
	mIsMatchWithoutKeyword(FALSE),
	mIsEnableReverse(FALSE),
	mIsNotifyUpdate(FALSE),
	mIsExpandMacro(FALSE),
	mActionType(2),
	mAfterCommandParam(_T("$value"))
{
}

SimpleDictParam::~SimpleDictParam()
{
}

bool SimpleDictParam::operator == (const SimpleDictParam& rhs) const
{
	if (&rhs == this) {
		return true;
	}

	return mName == rhs.mName &&
	       mDescription == rhs.mDescription &&
	       mFilePath == mFilePath &&
	       mSheetName == mSheetName &&
	       mRangeFront == mRangeFront &&
	       mRangeBack == mRangeBack &&
	       mIsFirstRowHeader == mIsFirstRowHeader && 
	       mIsMatchWithoutKeyword == mIsMatchWithoutKeyword &&
	       mIsEnableReverse == mIsEnableReverse &&
	       mIsNotifyUpdate == mIsNotifyUpdate &&
	       mIsExpandMacro == mIsExpandMacro &&
	       mActionType == mActionType &&
	       mAfterCommandName == mAfterCommandName &&
	       mAfterFilePath == mAfterFilePath &&
	       mAfterCommandParam == mAfterCommandParam;
}

bool SimpleDictParam::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	// Note: nameは上位で書き込みを行っているのでここではしない
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("FilePath"), mFilePath);
	entry->Set(_T("SheetName"), mSheetName);
	entry->Set(_T("Range"), mRangeFront);
	entry->Set(_T("RangeBack"), mRangeBack);
	entry->Set(_T("IsFirstRowHeader"), (bool)mIsFirstRowHeader);
	entry->Set(_T("IsMatchWithoutKeyword"), (bool)mIsMatchWithoutKeyword);
	entry->Set(_T("IsEnableReverse"), (bool)mIsEnableReverse);
	entry->Set(_T("IsNotifyUpdate"), (bool)mIsNotifyUpdate);
	entry->Set(_T("IsExpandMacro"), (bool)mIsExpandMacro);
	entry->Set(_T("aftertype"), mActionType);
	entry->Set(_T("aftercommand"), mAfterCommandName);
	entry->Set(_T("afterfilepath"), mAfterFilePath);
	entry->Set(_T("afterparam"), mAfterCommandParam);

	return true;
}

bool SimpleDictParam::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	mFilePath = entry->Get(_T("FilePath"), _T(""));
	mSheetName = entry->Get(_T("SheetName"), _T(""));
	mRangeFront = entry->Get(_T("Range"), _T(""));
	mRangeBack = entry->Get(_T("RangeBack"), _T(""));
	mIsFirstRowHeader = entry->Get(_T("IsFirstRowHeader"), false);
	mIsMatchWithoutKeyword = entry->Get(_T("IsMatchWithoutKeyword"), true);
	mIsEnableReverse = entry->Get(_T("IsEnableReverse"), false);
	mIsNotifyUpdate = entry->Get(_T("IsNotifyUpdate"), false);
	mIsExpandMacro = entry->Get(_T("IsExpandMacro"), false);

	mActionType = entry->Get(_T("aftertype"), 2);
	mAfterCommandName = entry->Get(_T("aftercommand"), _T(""));
	mAfterFilePath = entry->Get(_T("afterfilepath"), _T(""));
	mAfterCommandParam = entry->Get(_T("afterparam"), _T("$value"));

	return true;
}

void SimpleDictParam::swap(SimpleDictParam& rhs)
{
	std::swap(mName, rhs.mName);
	std::swap(mDescription, rhs.mDescription);
	std::swap(mFilePath, rhs.mFilePath);
	std::swap(mSheetName, rhs.mSheetName);
	std::swap(mRangeFront, rhs.mRangeFront);
	std::swap(mRangeBack, rhs.mRangeBack);
	std::swap(mIsFirstRowHeader, rhs.mIsFirstRowHeader); 
	std::swap(mIsMatchWithoutKeyword, rhs.mIsMatchWithoutKeyword);
	std::swap(mIsEnableReverse, rhs.mIsEnableReverse);
	std::swap(mIsNotifyUpdate, rhs.mIsNotifyUpdate);
	std::swap(mIsExpandMacro, rhs.mIsExpandMacro);
	std::swap(mActionType, rhs.mActionType);
	std::swap(mAfterCommandName, rhs.mAfterCommandName);
	std::swap(mAfterFilePath, rhs.mAfterFilePath);
	std::swap(mAfterCommandParam, rhs.mAfterCommandParam);
}

}
}
}



