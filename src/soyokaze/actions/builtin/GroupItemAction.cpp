#include "pch.h"
#include "GroupItemAction.h"
#include "RunCommandAction.h"
#include "ExecuteAction.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/web/OpenURLAction.h"
#include "actions/core/ActionParameter.h"

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::core;
using GroupItem = launcherapp::commands::group::GroupItem;
using GroupItemType = launcherapp::commands::group::GroupItemType;

namespace launcherapp { namespace actions { namespace builtin {

struct GroupItemAction::PImpl
{
	CString mParentName;
	GroupItem mItem;
	HOTKEY_ATTR mHotkeyAttr;
};

GroupItemAction::GroupItemAction(const CString& parentName, const GroupItem& item,
	const HOTKEY_ATTR& hotkeyAttr) : in(new PImpl)
{
	in->mParentName = parentName;
	in->mItem = item;
	in->mHotkeyAttr = hotkeyAttr;
}

GroupItemAction::~GroupItemAction()
{
}

CString GroupItemAction::GetDisplayName()
{
	return _T("実行");
}

bool GroupItemAction::Perform(Parameter* param, String* errMsg)
{
	if (in->mItem.mType == GroupItemType::Command) {
		RunCommandAction action(in->mParentName, in->mItem.mItemName, in->mHotkeyAttr);
		action.EnableWait(in->mItem.mIsWait);
		action.SetParameterTemplate(in->mItem.mParam);
		return action.Perform(param, errMsg);
	}

	if (in->mItem.mType == GroupItemType::Path) {
		CString path = in->mItem.mItemName;
		CString arguments = in->mItem.mParam;
		CString workDir = in->mItem.mWorkDir;
		ExpandArguments(path, param);
		ExpandArguments(arguments, param);
		ExpandArguments(workDir, param);
		ExpandMacros(path);
		ExpandMacros(arguments);
		ExpandMacros(workDir);

		ExecuteAction action(path, arguments, workDir, in->mItem.mShowType);
		RefPtr<Parameter> paramSub(param->Clone(), false);
		auto namedParam = GetNamedParameter(paramSub);
		namedParam->SetNamedParamBool(_T("WAIT"), in->mItem.mIsWait);
		return action.Perform(paramSub, errMsg);
	}

	// URLはブラウザへの起動要求までしか完了を検知できないため、待機しない
	CString url = in->mItem.mItemName;
	ExpandArguments(url, param);
	ExpandMacros(url);
	launcherapp::actions::web::OpenURLAction action(url);
	return action.Perform(param, errMsg);
}

}}}
