#pragma once

#include "core/CommandIF.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace watchpath {

class WatchPathCommand : public soyokaze::core::Command
{
public:
	WatchPathCommand();
	virtual ~WatchPathCommand();

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

	bool Load(CommandFile* cmdFile, void* entry_);

	uint32_t AddRef() override;
	uint32_t Release() override;

	static CString GetType();

	static bool NewDialog(const Parameter* param);
	
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}

