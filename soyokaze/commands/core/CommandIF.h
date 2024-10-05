#pragma once

#include "commands/core/UnknownIF.h"
#include "commands/core/CommandEntryIF.h"
#include "commands/core/CommandParameter.h"
#include "matcher/Pattern.h"

class CommandHotKeyAttribute;

namespace launcherapp {
namespace core {

class Command : virtual public UnknownIF
{
protected:
	using Parameter = launcherapp::core::CommandParameter;
public:
	virtual ~Command() {}

	virtual CString GetName() = 0;
	virtual CString GetDescription() = 0;
	virtual CString GetGuideString() = 0;
	// コマンド種類を表す表示名称
	virtual CString GetTypeDisplayName() = 0;
	virtual BOOL Execute(Parameter* param) = 0;
	virtual CString GetErrorString() = 0;
	virtual HICON GetIcon() = 0;
	virtual int Match(Pattern* pattern) = 0;
	virtual bool IsEditable() = 0;
	virtual bool IsDeletable() = 0;
	virtual int EditDialog(HWND parent) = 0;
	virtual bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) = 0;

	/**
	 *  @brief 優先順位の重みづけを使用するか?
	 *  @true true:優先順位の重みづけを使用する false:使用しない
	 */
	virtual bool IsPriorityRankEnabled() = 0;

	virtual Command* Clone() = 0;

	virtual bool Save(CommandEntryIF* entry) = 0;
	virtual bool Load(CommandEntryIF* entry) = 0;
};


} // end of namespace core
} // end of namespace launcherapp
