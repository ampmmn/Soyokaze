#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/CommandQueryItem.h"	
#include "commands/simple_dict/SimpleDictParam.h"
#include <memory>

namespace soyokaze {
namespace commands {
namespace simple_dict {

class CommandUpdateListenerIF;

class SimpleDictCommand : public soyokaze::core::Command
{
public:
	SimpleDictCommand();
	virtual ~SimpleDictCommand();

	void AddListener(CommandUpdateListenerIF* listener);
	void RemoveListener(CommandUpdateListenerIF* listener);

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

	static bool NewDialog(const Parameter* param, SimpleDictCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, SimpleDictCommand** newCmdPtr);

	const SimpleDictParam& GetParam();

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace soyokaze

