// あ
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
	virtual CString GetName() = 0;
	virtual CString GetDescription() = 0;
	virtual CString GetGuideString() = 0;
	// コマンド種類を表す表示名称
	virtual CString GetTypeDisplayName() = 0;
	virtual bool CanExecute() = 0;
	virtual BOOL Execute(Parameter* param) = 0;
	virtual CString GetErrorString() = 0;
	virtual HICON GetIcon() = 0;
	virtual int Match(Pattern* pattern) = 0;
	// 完全一致かつ候補が一つのときに自動実行を許すか
	virtual bool IsAllowAutoExecute() = 0;

	// ホットキー設定を取得する
	// 戻り値は ホットキーを設定する機能を有する場合はtrue、そうでなければfalse
	virtual bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) = 0;

	virtual Command* Clone() = 0;

	virtual bool Save(CommandEntryIF* entry) = 0;
	virtual bool Load(CommandEntryIF* entry) = 0;
};


} // end of namespace core
} // end of namespace launcherapp
