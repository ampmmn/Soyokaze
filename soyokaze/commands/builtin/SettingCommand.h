#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace soyokaze {
namespace commands {
namespace builtin {

class SettingCommand : public BuiltinCommandBase
{
public:
	SettingCommand(LPCTSTR name = nullptr);
	virtual ~SettingCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	soyokaze::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

protected:
	bool mIsExecuting;
	// 前回表示時のページ階層を表すパンくずリスト
	CString mLastBreakCrumbs;
};

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

