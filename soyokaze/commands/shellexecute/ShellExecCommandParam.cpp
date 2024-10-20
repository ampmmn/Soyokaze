#include "pch.h"
#include "ShellExecCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace shellexecute {


int ATTRIBUTE::GetShowType() const
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CommandParam::CommandParam() :
	mIsRunAsAdmin(FALSE),
	mIsUse0(FALSE),
	mIsShowArgDialog(FALSE),
	mIsUseDescriptionForMatching(FALSE)
{
}


CommandParam::CommandParam(const CommandParam& rhs)
{
	mName = rhs.mName;
	mDescription = rhs.mDescription;
	mNormalAttr = rhs.mNormalAttr;
	mNoParamAttr = rhs.mNoParamAttr;
	mIsRunAsAdmin = rhs.mIsRunAsAdmin;
	mIsUse0 = rhs.mIsUse0;
	mIsShowArgDialog = rhs.mIsShowArgDialog;
	mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
	mIconData = rhs.mIconData;
	mHotKeyAttr = rhs.mHotKeyAttr;
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (&rhs != this) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mNormalAttr = rhs.mNormalAttr;
		mNoParamAttr = rhs.mNoParamAttr;
		mIsRunAsAdmin = rhs.mIsRunAsAdmin;
		mIsUse0 = rhs.mIsUse0;
		mIsShowArgDialog = rhs.mIsShowArgDialog;
		mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
		mIconData = rhs.mIconData;
		mHotKeyAttr = rhs.mHotKeyAttr;
	}
	return *this;
}

bool CommandParam::Save(CommandEntryIF* entry) const
{
	ASSERT(entry);

	entry->Set(_T("description"), mDescription);
	entry->Set(_T("runas"), mIsRunAsAdmin ? 1 : 0);
	entry->Set(_T("isShowArgInput"), mIsShowArgDialog);

	entry->Set(_T("path"), mNormalAttr.mPath);
	entry->Set(_T("dir"), mNormalAttr.mDir);
	entry->Set(_T("parameter"), mNormalAttr.mParam);
	entry->Set(_T("show"), mNormalAttr.mShowType);

	entry->Set(_T("path0"), mNoParamAttr.mPath);
	entry->Set(_T("dir0"), mNoParamAttr.mDir);
	entry->Set(_T("parameter0"), mNoParamAttr.mParam);
	entry->Set(_T("show0"), mNoParamAttr.mShowType);

	entry->SetBytes(_T("IconData"),
	                (const uint8_t*)mIconData.data(), mIconData.size());
	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	mIsRunAsAdmin = (entry->Get(_T("runas"), 0) != 0);

	mNormalAttr.mPath = entry->Get(_T("path"), _T(""));
	mNormalAttr.mDir = entry->Get(_T("dir"), _T(""));
	mNormalAttr.mParam = entry->Get(_T("parameter"), _T(""));
	mNormalAttr.mShowType = entry->Get(_T("show"), mNormalAttr.mShowType);

	mNoParamAttr.mPath = entry->Get(_T("path0"), _T(""));
	mNoParamAttr.mDir = entry->Get(_T("dir0"), _T(""));
	mNoParamAttr.mParam = entry->Get(_T("parameter0"), _T(""));
	mNoParamAttr.mShowType = entry->Get(_T("show0"), mNoParamAttr.mShowType);
	mIsShowArgDialog = entry->Get(_T("isShowArgInput"), 0);

	size_t len = entry->GetBytesLength(_T("IconData"));
	if (len != CommandEntryIF::NO_ENTRY) {
		mIconData.resize(len);
		entry->GetBytes(_T("IconData"), (uint8_t*)mIconData.data(), len);
	}

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 

	return true;
}

}
}
}

