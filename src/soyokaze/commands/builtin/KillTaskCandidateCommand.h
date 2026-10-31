#pragma once

#include "commands/common/AdhocCommandBase.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace builtin {

class KillTaskCandidateCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	/** プロセス終了候補を生成する */
	KillTaskCandidateCommand(
		DWORD processId,
		const CString& imageName,
		const CString& imagePath,
		const CString& commandLine,
		bool isPermissionDenied,
		uint64_t creationTime,
		bool isConfirmBeforeRun);
	/** プロセス終了候補を破棄する */
	virtual ~KillTaskCandidateCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(KillTaskCandidateCommand)

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
