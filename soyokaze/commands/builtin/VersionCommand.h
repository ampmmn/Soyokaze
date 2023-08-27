#pragma once

#include "core/CommandIF.h"

namespace soyokaze {
namespace commands {
namespace builtin {

class VersionCommand : public soyokaze::core::Command
{
public:
	VersionCommand(LPCTSTR name = nullptr);
	virtual ~VersionCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;
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

	static CString GetType();

protected:
	CString mName;
	bool mIsExecuting;
	uint32_t mRefCount;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

