#pragma once

#include "core/CommandIF.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace align_window {

class AlignWindowCommand : public soyokaze::core::Command
{
public:
	AlignWindowCommand();
	virtual ~AlignWindowCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(const Parameter& param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool IsEditable() override;
	int EditDialog(const Parameter* param) override;
	bool IsPriorityRankEnabled() override;
	soyokaze::core::Command* Clone() override;

	bool Save(CommandFile* cmdFile) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

	static CString GetType();

	static bool NewDialog(const Parameter* param, AlignWindowCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, AlignWindowCommand** newCmdPtr);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace align_window
} // end of namespace commands
} // end of namespace soyokaze

