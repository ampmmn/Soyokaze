#include "pch.h"
#include "ShowPropertiesAction.h"
#include "commands/common/Clipboard.h"
#include "actions/core/ActionParameter.h"
#include "commands/core/CommandRepository.h"
#include "utility/LastErrorString.h"

namespace launcherapp { namespace actions { namespace builtin {

using namespace launcherapp::actions::core;

class ShowpropertiesTarget : public ExecutionTarget
{
public:
	ShowpropertiesTarget(const CString& path) : mFullPath(path) {}

	CString GetPath(Parameter*) override { return mFullPath; }

	// 以下は使用しない
	CString GetParameter(Parameter*) override { return _T(""); }
	CString GetWorkDir(Parameter*) override { return _T(""); }
	int GetShowType(Parameter*) override { return SW_SHOW; }

	CString mFullPath;
};



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct ShowPropertiesAction::PImpl
{
	std::unique_ptr<ExecutionTarget> mTarget;
};

ShowPropertiesAction::ShowPropertiesAction(const CString& filePath) : in(new PImpl)
{
	in->mTarget.reset(new ShowpropertiesTarget(filePath));
}

ShowPropertiesAction::ShowPropertiesAction(ExecutionTarget* target) : in(new PImpl)
{
	in->mTarget.reset(target);
}

// Action
// アクションの内容を示す名称
CString ShowPropertiesAction::GetDisplayName()
{
	return _T("プロパティ");
}

// アクションを実行する
bool ShowPropertiesAction::Perform(Parameter* param, String* errMsg)
{
	UNREFERENCED_PARAMETER(param);
	
	if (SHObjectProperties(nullptr, SHOP_FILEPATH, in->mTarget->GetPath(param), nullptr) == FALSE) {
		if (errMsg) {
			LastErrorString msg(GetLastError());
			UTF2UTF((CString)(LPCTSTR)msg, *errMsg);
		}
		return false;
	}

	return true;
}

}}}

