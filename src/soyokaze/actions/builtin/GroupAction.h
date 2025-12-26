#pragma once

#include "actions/core/ActionBase.h"
#include "hotkey/HotKeyAttribute.h"

class HOTKEY_ATTR;

namespace launcherapp { namespace actions { namespace builtin {

// 他のアクションをまとめて実行するアクション
class GroupAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	GroupAction(const CString& parentName, const HOTKEY_ATTR& hotkeyAttr);
	~GroupAction();

	void AddAction(Action* action);
	void EnableConfirm(bool shouldConfirm);
	void StopIfErrorOccured(bool shouldStop);
	void SetRepeats(uint32_t repeats);
	void EnablePassParam(bool shouldPassParam);
	
// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;
	// ガイド欄などに表示するかどうか
	bool IsVisible() override;

private:
	bool Confirm(Parameter* param);
	bool BuildSubParameter(Parameter* param, Parameter** paramSub);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}
