#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace mainwindow {

class SetTextAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	SetTextAction(LPCTSTR dispalayName, LPCTSTR text);
	~SetTextAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	CString mDisplayName;
	CString mText;
};



}}}

