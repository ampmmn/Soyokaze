#include "pch.h"
#include "CallbackAction.h"

namespace launcherapp { namespace actions { namespace builtin {

	struct CallbackAction::PImpl
	{
		LPCALLBACKFUNC mCallbackFunction{ nullptr };
		CString mDisplayName;
		void* mUserParam{ nullptr };
	};

CallbackAction::CallbackAction(LPCTSTR dispName, LPCALLBACKFUNC func, void* userParam) : in(new PImpl())
{
	in->mDisplayName = dispName;
	in->mCallbackFunction = func;
	in->mUserParam = userParam;
}

CallbackAction::~CallbackAction()
{
}

// Action
// アクションの内容を示す名称
CString CallbackAction::GetDisplayName()
{
	return in->mDisplayName;
}

// アクションを実行する
bool CallbackAction::Perform(Parameter* param, String* errMsg)
{
	if (in->mCallbackFunction == nullptr) {
		return false;
	}
	return in->mCallbackFunction(param, errMsg, in->mUserParam);
}

}}}

