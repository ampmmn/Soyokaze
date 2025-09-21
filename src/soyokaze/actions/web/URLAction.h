#pragma once

#include "actions/core/ActionBase.h"

namespace launcherapp { namespace actions { namespace web {

// 指定されたURLをデフォルトのブラウザで開くアクション
class URLAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	URLAction(const CString& url);
	URLAction(const CString& displayName, const CString& url);
	~URLAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	CString mDisplayName;
	CString mURL;
};



}}}

