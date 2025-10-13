#include "pch.h"
#include "PyExtensionCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/validation/CommandEditValidation.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace py_extension {

using CommandParamErrorCode = launcherapp::commands::validation::CommandParamErrorCode;

bool CommandParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("script"), mScript);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mScript = entry->Get(_T("script"), _T(""));

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 

	return true;
}

bool CommandParam::IsValid(LPCTSTR orgName, int* errCode) const
{
	// 名前チェック
	if (launcherapp::commands::validation::IsValidCommandName(mName, orgName, errCode) == false) {
		return false;
	}

	if (mScript.IsEmpty()) {
		*errCode = CommandParamErrorCode::PyExtension_ScriptIsEmpty;
		return false;
	}

	*errCode = CommandParamErrorCode::Common_NoError;
	return true;
}

}}} // end of namespace launcherapp::commands::py_extension

