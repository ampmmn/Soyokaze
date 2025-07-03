#include "pch.h"
#include "ShellExecCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace shellexecute {


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

void ATTRIBUTE::SetShowType(int type)
{
	switch(type) {
	case SW_MAXIMIZE:
		mShowType = 1;
		return;
	case SW_SHOWMINIMIZED:
		mShowType = 2;
		return;
	default:
		mShowType = 0;
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CommandParam::CommandParam() :
	mIsRunAsAdmin(FALSE),
	mIsUse0(FALSE),
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
	mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
	mIconData = rhs.mIconData;
	mEnviron = rhs.mEnviron;
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
		mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
		mIconData = rhs.mIconData;
		mEnviron = rhs.mEnviron;
		mHotKeyAttr = rhs.mHotKeyAttr;
	}
	return *this;
}

bool CommandParam::Save(CommandEntryIF* entry) const
{
	ASSERT(entry);

	entry->Set(_T("description"), mDescription);
	entry->Set(_T("runas"), mIsRunAsAdmin ? 1 : 0);

	entry->Set(_T("isuse0"), mIsUse0);
	entry->Set(_T("isusedescriptionformatching"), mIsUseDescriptionForMatching);

	entry->Set(_T("path"), mNormalAttr.mPath);
	entry->Set(_T("dir"), mNormalAttr.mDir);
	entry->Set(_T("parameter"), mNormalAttr.mParam);
	entry->Set(_T("show"), mNormalAttr.GetShowType());

	entry->Set(_T("path0"), mNoParamAttr.mPath);
	entry->Set(_T("dir0"), mNoParamAttr.mDir);
	entry->Set(_T("parameter0"), mNoParamAttr.mParam);
	entry->Set(_T("show0"), mNoParamAttr.GetShowType());

	entry->SetBytes(_T("IconData"),
	                (const uint8_t*)mIconData.data(), mIconData.size());

	CString key;
	int index = 1;
	for (const auto& item : mEnviron) {
		if (item.first.IsEmpty()) {
			continue;
		}
		key.Format(_T("envname%d"), index);
		entry->Set(key, item.first);
		key.Format(_T("envvalue%d"), index);
		entry->Set(key, item.second);
		index++;
	}

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	mIsRunAsAdmin = (entry->Get(_T("runas"), 0) != 0);

	mIsUse0 = entry->Get(_T("isuse0"), FALSE);
	mIsUseDescriptionForMatching = entry->Get(_T("isusedescriptionformatching"), FALSE);

	mNormalAttr.mPath = entry->Get(_T("path"), _T(""));
	mNormalAttr.mDir = entry->Get(_T("dir"), _T(""));
	mNormalAttr.mParam = entry->Get(_T("parameter"), _T(""));
	mNormalAttr.SetShowType(entry->Get(_T("show"), SW_SHOW));

	mNoParamAttr.mPath = entry->Get(_T("path0"), _T(""));
	mNoParamAttr.mDir = entry->Get(_T("dir0"), _T(""));
	mNoParamAttr.mParam = entry->Get(_T("parameter0"), _T(""));
	mNoParamAttr.SetShowType(entry->Get(_T("show0"), SW_SHOW));

	size_t len = entry->GetBytesLength(_T("IconData"));
	if (len != CommandEntryIF::NO_ENTRY) {
		mIconData.resize(len);
		entry->GetBytes(_T("IconData"), (uint8_t*)mIconData.data(), len);
	}

	// 環境変数
	std::map<CString, CString> envMap;
	CString key;
	CString envName;
	CString envValue;
	for (int index = 1;; ++index) {
		key.Format(_T("envname%d"), index);
		envName = entry->Get(key, _T(""));
		if (envName.IsEmpty()) {
			break;
		}
		key.Format(_T("envvalue%d"), index);
		envValue = entry->Get(key, _T(""));
		envMap[envName] = envValue;
	}
	mEnviron.swap(envMap);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 

	return true;
}

}}} // end of namespace launcherapp::commands::shellexecute

