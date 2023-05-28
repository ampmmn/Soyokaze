#pragma once

#include "core/CommandIF.h"

class MainDirCommand : public soyokaze::core::Command
{
public:
	MainDirCommand();
	virtual ~MainDirCommand();

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
	uint32_t mRefCount;
};

