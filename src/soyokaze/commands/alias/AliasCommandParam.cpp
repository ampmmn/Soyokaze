#include "pch.h"
#include "AliasCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/validation/CommandEditValidation.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace alias {

bool CommandParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("text"), mText);
	entry->Set(_T("pasteonly"), mIsPasteOnly);
	entry->Set(_T("allow_auto_execute"), mIsAllowAutoExecute);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mText = entry->Get(_T("text"), _T(""));
	mIsPasteOnly = entry->Get(_T("pasteonly"), 0);
	mIsAllowAutoExecute = entry->Get(_T("allow_auto_execute"), false);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 

	return true;
}

bool CommandParam::Validate(LPCTSTR orgName, CString& errMsg)
{
	// 名前チェック
	bool isNameValid =
	 	launcherapp::commands::validation::IsValidCommandName(mName, orgName, errMsg);
	if (isNameValid == false) {
		return false;
	}

	if (mText.IsEmpty()) {
		BOOL isOK = errMsg.LoadString(IDS_ERR_TEXTISEMPTY);
		if (isOK == FALSE) {
			spdlog::error("Failed to load string IDS_ERR_TEXTISEMPTY");
		}
		return false;
	}

	errMsg.Empty();
	return true;
}

}}} // end of namespace launcherapp::commands::builtin

