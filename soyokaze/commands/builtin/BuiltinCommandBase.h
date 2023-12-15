#pragma once

#include "core/CommandIF.h"

namespace soyokaze {
namespace commands {
namespace builtin {

class BuiltinCommandBase : public soyokaze::core::Command
{
public:
	BuiltinCommandBase(LPCTSTR name = nullptr);
	virtual ~BuiltinCommandBase();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	//BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(const Parameter* param) override;
	bool IsPriorityRankEnabled() override;
	//soyokaze::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

	virtual CString GetType() = 0;

protected:
	CString mName;
	CString mDescription;
	CString mError;
	uint32_t mRefCount;
};

}
}
}

