#include "pch.h"
#include "AliasCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"

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

}}} // end of namespace launcherapp::commands::builtin

