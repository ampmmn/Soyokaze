#include "pch.h"
#include "CommandEditValidation.h"
#include "commands/core/CommandRepository.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

bool IsValidCommandName(const CString& name, const CString& orgName, CString& errMsg)
{
	// 名前チェック
	if (name.IsEmpty()) {
		errMsg.LoadString(IDS_ERR_NAMEISEMPTY);
		return false;
	}

	auto cmdRepoPtr = launcherapp::core::CommandRepository::GetInstance();
	// 重複チェック
	if (name.CompareNoCase(orgName) != 0) {
		RefPtr<launcherapp::core::Command> cmd(cmdRepoPtr->QueryAsWholeMatch(name, false));
		if (cmd != nullptr) {
			errMsg.LoadString(IDS_ERR_NAMEALREADYEXISTS);
			return false;
		}
	}
	// 使えない文字チェック
	if (cmdRepoPtr->IsValidAsName(name) == false) {
		errMsg.LoadString(IDS_ERR_ILLEGALCHARCONTAINS);
		return false;
	}
	return true;
}

}
}
}

