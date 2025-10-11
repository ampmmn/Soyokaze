#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ExtraCandidateIF.h"
#include <memory>
#include <nlohmann/json.hpp>

namespace launcherapp {
namespace commands {
namespace vscode {

class CommandParam;

class VSCodeFileCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidate
{
private:
	VSCodeFileCommand(const CommandParam* param, const CString& displayName, const CString& localPath);
	virtual ~VSCodeFileCommand();

public:
	static bool Create(nlohmann::json& json, CommandParam* param, Command** cmd);

	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	launcherapp::core::Command* Clone() override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(VSCodeFileCommand)

public:
	static CString TypeDisplayName();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace vscode
} // end of namespace commands
} // end of namespace launcherapp

