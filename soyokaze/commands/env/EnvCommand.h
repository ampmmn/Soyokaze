#pragma once

#include "core/CommandIF.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace env {


class EnvCommand : public soyokaze::core::Command
{
public:
	EnvCommand(const CString& name, const CString& value);
	virtual ~EnvCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
	BOOL Execute() override;
	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(const Parameter* param) override;
	soyokaze::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace env
} // end of namespace commands
} // end of namespace soyokaze

