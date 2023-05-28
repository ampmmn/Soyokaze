#pragma once

#include "core/CommandIF.h"

class CommandRepository;

class RegistWinCommand : public soyokaze::core::Command
{
public:
	RegistWinCommand(CommandRepository* cmdMapPtr);
	virtual ~RegistWinCommand();

	CString GetName() override;
	CString GetDescription() override;
	BOOL Execute() override;
	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;
	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	struct PImpl;
	PImpl* in;
};

