#pragma once

#include "commands/core/CommandIF.h"
#include <memory>

class HOTKEY_ATTR;

namespace soyokaze {
namespace commands {
namespace filter {

class CommandParam;

class FilterCommand : public soyokaze::core::Command
{
public:
	FilterCommand();
	virtual ~FilterCommand();

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
	
public:
	FilterCommand& SetParam(const CommandParam& param);
	FilterCommand& GetParam(CommandParam& param);


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

