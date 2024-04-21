#pragma once

#include "commands/core/CommandIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace common {


class AdhocCommandBase : public launcherapp::core::Command
{
public:
	AdhocCommandBase(LPCTSTR name = _T(""), LPCTSTR description = _T(""));
	virtual ~AdhocCommandBase();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	//CString GetTypeDisplayName() override;
	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(const Parameter* param) override;
	bool IsPriorityRankEnabled() override;
	//launcherapp::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	uint32_t mRefCount;
	CString mName;
	CString mDescription;
	CString mErrMsg;
};


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

