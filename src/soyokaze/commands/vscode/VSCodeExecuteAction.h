#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace commands { namespace vscode {

class CommandParam;

class VSCodeExecuteAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	VSCodeExecuteAction(const CommandParam* param, const CString& path, bool isOpenInNewWindow);
	~VSCodeExecuteAction();

	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

protected:
	const CommandParam* mParam{nullptr};
	CString mFullPath;
	bool mIsOpenInNewWindow{false};
};


}}} // end of namespace launcherapp::commands::vscode

