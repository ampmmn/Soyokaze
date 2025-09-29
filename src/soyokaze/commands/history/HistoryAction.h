#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace commands { namespace history {

class HistoryAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	using Action = launcherapp::actions::core::Action;

	HistoryAction(launcherapp::actions::core::Action* realAction, const CString& keyword);
	~HistoryAction();

	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg = nullptr) override;
	// ガイド欄などに表示するかどうか
	bool IsVisible() override;

private:
	Action* mRealAction{nullptr};
	CString mKeyword;
};

}}} // end of namespace launcherapp::commands::history
