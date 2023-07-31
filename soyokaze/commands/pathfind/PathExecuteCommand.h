#pragma once

#include "core/CommandIF.h"

namespace soyokaze {
namespace commands {
namespace pathfind {



class PathExecuteCommand : public soyokaze::core::Command
{
public:
	PathExecuteCommand();
	virtual ~PathExecuteCommand();

	void SetFullPath(const CString& path);

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
	PImpl* in;
};


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze

