#pragma once

#include "commands/core/CommandIF.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace websearch {

class WebSearchCommand : public soyokaze::core::Command
{
public:
	WebSearchCommand();
	virtual ~WebSearchCommand();

	// 
	bool IsEnableShortcut() const;
	WebSearchCommand* CloneAsAdhocCommand(CString& searchWord);

// Comand
	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(const Parameter* param) override;
	bool IsPriorityRankEnabled() override;
	soyokaze::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, std::unique_ptr<WebSearchCommand>& newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, std::unique_ptr<WebSearchCommand>& newCmd);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace websearch
} // end of namespace commands
} // end of namespace soyokaze

