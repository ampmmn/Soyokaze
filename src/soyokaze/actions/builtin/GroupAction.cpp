#include "pch.h"
#include "GroupAction.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"
#include "SharedHwnd.h"
#include "resource.h"

using CommandRepository = launcherapp::core::CommandRepository;
using namespace launcherapp::commands::common;
using namespace launcherapp::actions::core;

namespace launcherapp { namespace actions { namespace builtin {

struct GroupAction::PImpl
{
	CString mParentCommandName;
	std::vector<Action*> mActions;
	uint32_t mModifierKeyState{0};
	uint32_t mRepeats{1};
	bool mShouldConfirm{false};
	bool mShouldStopIfErrorOccured{false};
	bool mShouldPassParam{false};
};


GroupAction::GroupAction(
	const CString& parentName,
	uint32_t modifierKeyState
) : in(new PImpl)
{
	in->mParentCommandName = parentName;
	in->mModifierKeyState = modifierKeyState;
}

GroupAction::~GroupAction()
{
	for (auto action : in->mActions) {
		action->Release();
	}
}

void GroupAction::AddAction(Action* action)
{
	action->AddRef();
	in->mActions.push_back(action);
}

void GroupAction::EnableConfirm(bool shouldConfirm)
{
	in->mShouldConfirm = shouldConfirm;
}

void GroupAction::StopIfErrorOccured(bool shouldStop)
{
	in->mShouldStopIfErrorOccured = shouldStop;
}
	
void GroupAction::SetRepeats(uint32_t repeats)
{
	in->mRepeats = repeats;
	if (in->mRepeats == 0) {
		in->mRepeats = 1;
	}
}

void GroupAction::EnablePassParam(bool shouldPassParam)
{
	in->mShouldPassParam = shouldPassParam;
}

// アクションの内容を示す名称
CString GroupAction::GetDisplayName()
{
	return _T("実行");
}

// アクションを実行する
bool GroupAction::Perform(Parameter* param, String* errMsg)
{
	// 実行前の確認
	if (Confirm(param) == false) {
		spdlog::info("GroupAction cancelled.");
		return false;
	}

	// 履歴に登録する
	auto namedParam = GetNamedParameter(param);
	if (param->HasParameter() && namedParam->GetNamedParamBool(_T("RunAsHistory")) == false) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param->GetWholeString());
	}

	// 実行する子アクションに渡すためのパラメータオブジェクトを作る
	RefPtr<Parameter> paramSub;
	BuildSubParameter(param, (Parameter**)&paramSub);

	bool isOK = true;

	// アクションを実行
	for (uint32_t round = 0; round < in->mRepeats; ++round) {
		for (auto action : in->mActions) {
			if (action->Perform(paramSub, errMsg) == false) {
				isOK = false;
			}
			if (in->mShouldStopIfErrorOccured) {
				return isOK;
			}
		}
	}

	return isOK;
}

bool GroupAction::Confirm(Parameter* param)
{
	if (in->mShouldConfirm == false) {
		// 実行前の確認は不要
		return true;
	}

	// 初回に(必要に応じて)実行確認を行う
	auto namedParam = GetNamedParameter(param);
	bool isConfirmed = namedParam->GetNamedParamBool(_T("CONFIRMED"));
	if (isConfirmed) {
		// 確認済
		return true;
	}

	CString msg;
	msg.Format(IDS_CONFIRMRUNGROUP, (LPCTSTR)in->mParentCommandName);

	SharedHwnd sharedHwnd;

	CString caption((LPCTSTR)IDS_TITLE_CONFIRMRUNGROUP);
	int n = MessageBox(sharedHwnd.GetHwnd(), msg, caption, MB_YESNO);
	if (n != IDYES) {
		// キャンセル
		return false;
	}

	return true;
}


bool GroupAction::BuildSubParameter(Parameter* param, Parameter** paramOut)
{
	RefPtr<Parameter> paramSub(param->Clone(), false);
	auto namedParam = GetNamedParameter(paramSub);
	namedParam->SetNamedParamBool(_T("CONFIRMED"), true);

	*paramOut = paramSub.release();
	return true;
}

}}}

