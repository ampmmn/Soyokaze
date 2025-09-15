#pragma once

#include "actions/core/Action.h"

namespace launcherapp { namespace actions { namespace core {

class ActionBase : virtual public Action
{
protected:
	using Parameter = launcherapp::actions::core::Parameter;
public:
	ActionBase();
	virtual ~ActionBase();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** obj) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	uint32_t mRefCount{1};
};



}}}

