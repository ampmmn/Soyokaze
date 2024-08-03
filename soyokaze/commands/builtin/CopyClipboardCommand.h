#pragma once

#include "commands/builtin/BuiltinCommandBase.h"

namespace launcherapp {
namespace commands {
namespace builtin {

class CopyClipboardCommand : public BuiltinCommandBase
{
public:
	CopyClipboardCommand(LPCTSTR name = nullptr);
	CopyClipboardCommand(const CopyClipboardCommand& rhs);
	virtual ~CopyClipboardCommand();

	BOOL Execute(const Parameter& param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

	CString GetType() override;
	static CString TYPE;

	virtual void OnCopy(const CString& str);

public:
	// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
	DECLARE_BUILTINCOMMAND(CopyClipboardCommand)
};

}
}
}
