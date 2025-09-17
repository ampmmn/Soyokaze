#include "pch.h"
#include "NullAction.h"

namespace launcherapp { namespace actions { namespace builtin {

// Action
// アクションの内容を示す名称
CString NullAction::GetDisplayName()
{
	return _T("");
}

// アクションを実行する
bool NullAction::Perform(Parameter* param, String* errMsg)
{
	// なにもしない
	return true;
}

}}}

