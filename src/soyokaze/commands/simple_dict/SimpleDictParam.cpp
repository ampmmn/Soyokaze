#include "pch.h"
#include "SimpleDictParam.h"
#include "commands/core/CommandEntryIF.h"
#include "utility/SHA1.h"

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
	       mFilePath == rhs.mFilePath &&
	       mSheetName == rhs.mSheetName &&
	       mRangeFront == rhs.mRangeFront &&
	       mRangeBack == rhs.mRangeBack &&
	       mRangeValue2 == rhs.mRangeValue2 &&
	       mNameFormat == rhs.mNameFormat &&
	       mDescriptionFormat == rhs.mDescriptionFormat &&
	       mIsFirstRowHeader == rhs.mIsFirstRowHeader && 
	       mIsMatchWithoutKeyword == rhs.mIsMatchWithoutKeyword &&
	       mIsEnableReverse == rhs.mIsEnableReverse &&
	       mIsNotifyUpdate == rhs.mIsNotifyUpdate &&
	       mIsExpandMacro == rhs.mIsExpandMacro &&
	       mActionType == rhs.mActionType &&
	       mAfterCommandName == rhs.mAfterCommandName &&
	       mAfterFilePath == rhs.mAfterFilePath &&
	       mAfterCommandParam == rhs.mAfterCommandParam;
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
	entry->Set(_T("RangeValue2"), mRangeValue2);
	entry->Set(_T("NameFormat"), mNameFormat);
	entry->Set(_T("DescriptionFormat"), mDescriptionFormat);
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
	mRangeValue2 = entry->Get(_T("RangeValue2"), _T(""));
	mNameFormat = entry->Get(_T("NameFormat"), _T(""));
	mDescriptionFormat = entry->Get(_T("DescriptionFormat"), _T(""));
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

CString SimpleDictParam::GetIdentifier() const
{
	auto tmp = mFilePath + mSheetName + mRangeFront + mRangeBack + mRangeValue2 + (mIsFirstRowHeader ? _T("T") : _T("F"));

	SHA1 sha1;
	sha1.Add(tmp);
	return sha1.Finish();
}

void SimpleDictParam::swap(SimpleDictParam& rhs)
{
	std::swap(mName, rhs.mName);
	std::swap(mDescription, rhs.mDescription);
	std::swap(mFilePath, rhs.mFilePath);
	std::swap(mSheetName, rhs.mSheetName);
	std::swap(mRangeFront, rhs.mRangeFront);
	std::swap(mRangeBack, rhs.mRangeBack);
	std::swap(mRangeValue2, rhs.mRangeValue2);
	std::swap(mNameFormat, rhs.mNameFormat);
	std::swap(mDescriptionFormat, rhs.mDescriptionFormat);
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



