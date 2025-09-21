#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace vmware {

// 指定されたVMを開くアクション
class RunVMXAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	RunVMXAction::RunVMXAction(const CString& vmName, const CString& vmxFilePath, int showType);
	~RunVMXAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	bool IsVMLocked();

private:
	CString mVMDisplayName;
	CString mVMXFilePath;
	int mShowType;
	CString mURL;
};



}}}

