#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class SettingCommand : public BuiltinCommandBase
{
public:
	SettingCommand(LPCTSTR name = nullptr);
	SettingCommand(const SettingCommand& rhs);
	virtual ~SettingCommand();

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;


	LRESULT OnCallbackExecute();

	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(SettingCommand)

protected:
	bool mIsExecuting;
	// 前回表示時のページ階層を表すパンくずリスト
	CString mLastBreakCrumbs;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

