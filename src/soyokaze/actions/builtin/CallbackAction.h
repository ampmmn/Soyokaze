#pragma once

#include "actions/core/ActionBase.h"
#include <memory>
#include <functional>

// 指定された関数ポインタ経由で関数をよぶアクション
namespace launcherapp { namespace actions { namespace builtin {

class CallbackAction : virtual public launcherapp::actions::core::ActionBase
{
public:
	typedef std::function<bool(Parameter*, String*)> LPCALLBACKFUNC;

public:
	CallbackAction(LPCTSTR dispName, LPCALLBACKFUNC func);
	~CallbackAction();

// Action
	// アクションの内容を示す名称
	CString GetDisplayName() override;
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};



}}}

