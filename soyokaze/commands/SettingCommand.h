#pragma once

#include "core/CommandIF.h"

class SettingCommand : public soyokaze::core::Command
{
public:
	SettingCommand(LPCTSTR name = nullptr);
	virtual ~SettingCommand();

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

	static CString GetType();

protected:
	CString mName;
	uint32_t mRefCount;
};
