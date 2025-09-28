#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace builtin {

// 指定したコマンドを実行するアクション
class RunCommandAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	RunCommandAction(const CString& parentName, const CString& commandName, uint32_t modifierKeyState = 0);
	~RunCommandAction();

	void EnableWait(bool shouldWait);
	
// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	bool VerifyCircularReference(Parameter* param, String* errMsg);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}

