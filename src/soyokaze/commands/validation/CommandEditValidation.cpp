#include "pch.h"
#include "CommandEditValidation.h"
#include "commands/core/CommandRepository.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace validation {

bool IsValidCommandName(const CString& name, const CString& orgName, CString& errMsg)
{
	int errCode;
	if (IsValidCommandName(name, orgName, &errCode) != false) {
		errMsg.Empty();
		return true;
	}

	BOOL isOK = TRUE;
	if (errCode == Common_NoName) {
		isOK = errMsg.LoadString(IDS_ERR_NAMEISEMPTY);
		ASSERT(isOK);
		return false;
	}
	else if (errCode == Common_NameAlreadyExists) {
		isOK = errMsg.LoadString(IDS_ERR_NAMEALREADYEXISTS);
		ASSERT(isOK);
		return false;
	}
	else if (errCode == Common_NameContainsIllegalChar) {
		isOK = errMsg.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
		ASSERT(isOK);
		return false;
	}

	// 上記のエラー以外は返らない想定
	ASSERT(0);
	return false;
}

bool IsValidCommandName(const CString& name, const CString& orgName, int* errCode)
{
	// 名前チェック
	if (name.IsEmpty()) {
		if (errCode) { *errCode = Common_NoName; }
		return false;
	}

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	// 重複チェック
	if (name.CompareNoCase(orgName) != 0) {
		RefPtr<launcherapp::core::Command> cmd(cmdRepoPtr->QueryAsWholeMatch(name, false));
		if (cmd != nullptr) {
			if (errCode) { *errCode = Common_NameAlreadyExists; }
			return false;
		}
	}
	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(name) == false) {
			if (errCode) { *errCode = Common_NameContainsIllegalChar; }
		return false;
	}
	return true;
}

}}}

