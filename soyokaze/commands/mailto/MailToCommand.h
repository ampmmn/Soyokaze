#pragma once

#include "core/CommandIF.h"

namespace soyokaze {
namespace commands {
namespace mailto {

class ExecuteHistory;


class MailToCommand : public soyokaze::core::Command
{
public:
	MailToCommand();
	virtual ~MailToCommand();

	CString GetName() override;
	CString GetDescription() override;
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
	PImpl* in;
};


} // end of namespace mailto
} // end of namespace commands
} // end of namespace soyokaze

