#include "pch.h"
#include "PlaceWindowInRegionParameter.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/validation/CommandEditValidation.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::validation;

namespace launcherapp { namespace commands { namespace place_window_in_region {

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mHotKeyAttr(rhs.mHotKeyAttr),
	mPlacement(rhs.mPlacement),
	mIsActivate(rhs.mIsActivate)
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
		mHotKeyAttr = rhs.mHotKeyAttr;
		mPlacement = rhs.mPlacement;
		mIsActivate = rhs.mIsActivate;
	}
	return *this;
}

bool CommandParam::IsValid(LPCTSTR orgName, int* errCode) const
{
	ASSERT(errCode);

	// 名前チェック
	if (launcherapp::commands::validation::IsValidCommandName(mName, orgName, errCode) == false) {
		return false;
	}

	*errCode = CommandParamErrorCode::Common_NoError;

	return true;
}

bool CommandParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("description"), mDescription);

	entry->SetBytes(_T("Placement"), (const uint8_t*)&mPlacement, sizeof(mPlacement));
	entry->Set(_T("IsActivate"), mIsActivate);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	CString name = entry->GetName();
	CString descriptionStr = entry->Get(_T("description"), _T(""));

	mName = name;
	entry->GetBytes(_T("Placement"), (uint8_t*)&mPlacement, sizeof(WINDOWPLACEMENT));
	entry->Get(_T("IsActivate"), mIsActivate);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 
	return true;
}

CRect CommandParam::GetRegionRect() const
{
	return mPlacement.rcNormalPosition;
}

}}}
