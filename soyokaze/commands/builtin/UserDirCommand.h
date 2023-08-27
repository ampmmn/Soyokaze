#pragma once

#include "core/CommandIF.h"

namespace soyokaze {
namespace commands {
namespace builtin {

class UserDirCommand : public soyokaze::core::Command
{
public:
	UserDirCommand(LPCTSTR name = nullptr);
	virtual ~UserDirCommand();

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
	uint32_t mRefCount;
};



} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

