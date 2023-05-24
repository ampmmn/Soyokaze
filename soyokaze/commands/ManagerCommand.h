#pragma once

#include "core/CommandIF.h"

class CommandRepository;

class ManagerCommand : public soyokaze::core::Command
{
public:
	ManagerCommand(CommandRepository* cmdMapPtr);
	virtual ~ManagerCommand();

	CString GetName() override;
	CString GetDescription() override;
	BOOL Execute() override;
	BOOL Execute(const std::vector<CString>& args) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	soyokaze::core::Command* Clone() override;
	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	CommandRepository* mCmdMapPtr;
	uint32_t mRefCount;
};

