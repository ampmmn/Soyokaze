#pragma once

#include "commands/core/CommandParameter.h"
#include "matcher/Pattern.h"

class CommandFile;

namespace launcherapp {
namespace core {

class CommandParameter;

class Command
{
protected:
	using Parameter = launcherapp::core::CommandParameter;
public:
	virtual ~Command() {}

	virtual CString GetName() = 0;
	virtual CString GetDescription() = 0;
	virtual CString GetGuideString() = 0;
	virtual CString GetTypeDisplayName() = 0;
	virtual BOOL Execute(const Parameter& param) = 0;
	virtual CString GetErrorString() = 0;
	virtual HICON GetIcon() = 0;
	virtual int Match(Pattern* pattern) = 0;
	virtual bool IsEditable() = 0;
	virtual int EditDialog(const CommandParameter* param = nullptr) = 0;

	/**
	 *  @brief 優先順位の重みづけを使用するか?
	 *  @true true:優先順位の重みづけを使用する false:使用しない
	 */
	virtual bool IsPriorityRankEnabled() = 0;

	virtual Command* Clone() = 0;

	virtual bool Save(CommandFile* cmdFile) = 0;

	virtual uint32_t AddRef() = 0;
	virtual uint32_t Release() = 0;

};


} // end of namespace core
} // end of namespace launcherapp
