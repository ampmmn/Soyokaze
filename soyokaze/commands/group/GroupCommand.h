#pragma once

#include "core/CommandIF.h"
#include <memory>

class HOTKEY_ATTR;

namespace soyokaze {
namespace commands {
namespace group {

class CommandParam;

// グループコマンド
class GroupCommand : public soyokaze::core::Command
{
	class Exception;
public:
	GroupCommand();
	virtual ~GroupCommand();

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

	static CString GroupCommand::GetType();

public:
	void SetParam(const CommandParam& param);

	void AddItem(LPCTSTR itemName, bool isWait);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace group
} // end of namespace commands
} // end of namespace soyokaze

