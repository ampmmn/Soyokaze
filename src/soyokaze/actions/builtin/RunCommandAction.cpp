#include "pch.h"
#include "RunCommandAction.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandRepository.h"
#include "actions/core/ActionParameter.h"

using CommandRepository = launcherapp::core::CommandRepository;
using namespace launcherapp::commands::common;
using namespace launcherapp::actions::core;

namespace launcherapp { namespace actions { namespace builtin {

struct RunCommandAction::PImpl
{
	CString mParentCommandName;
	CString mRunCommandName;
	uint32_t mModifierKeyState{0};
	bool mShouldWait{false};

};


RunCommandAction::RunCommandAction(
	const CString& parentName,
 	const CString& commandName,
	uint32_t modifierKeyState
) : in(new PImpl)
{
	in->mParentCommandName = parentName;
	in->mRunCommandName = commandName;
	in->mModifierKeyState = modifierKeyState;
}

RunCommandAction::~RunCommandAction()
{
}

void RunCommandAction::EnableWait(bool shouldWait)
{
	in->mShouldWait = shouldWait;
}

// アクションの内容を示す名称
CString RunCommandAction::GetDisplayName()
{
	return _T("実行");
}

// アクションを実行する
bool RunCommandAction::Perform(Parameter* param, String* errMsg)
{
	if (VerifyCircularReference(param, errMsg) == false) {
		return false;
	}

	// 呼び出し元に自分自身を追加(循環参照チェック用)
	CString parents;
	auto namedParam = GetNamedParameter(param);
	int len = namedParam->GetNamedParamStringLength(_T("PARENTS"));
	if (len > 0) {
		namedParam->GetNamedParamString(_T("PARENTS"), parents.GetBuffer(len), len);
		parents.ReleaseBuffer();
	}

	if (parents.IsEmpty() == FALSE) {
		parents += _T("/");
	}
	parents += in->mParentCommandName;


	RefPtr<Parameter> paramSub(param->Clone(), false);
	auto namedParamSub = GetNamedParameter(paramSub);
	namedParamSub->SetNamedParamString(_T("PARENTS"), parents);
	namedParamSub->SetNamedParamBool(_T("WAIT"), in->mShouldWait);

	// 実行対象のコマンドを取得
	auto cmdRepo = CommandRepository::GetInstance();
	RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(in->mRunCommandName, false));
	if (command.get() == nullptr) {
		if (errMsg) {
			*errMsg = "コマンドが見つかりません";
		}
		return false;
	}

	// コマンドからアクションを取得
	RefPtr<Action> action;
	if (command->GetAction(in->mModifierKeyState, &action) == false) {
		if (command->GetAction(0, &action) == false) {
			spdlog::error("Failed to get action.");
			return false;
		}
	}
	// アクションを実行する
	return action->Perform(paramSub, errMsg);
}

// 循環参照チェック
bool RunCommandAction::VerifyCircularReference(Parameter* param, String* errMsg)
{
	auto namedParam = GetNamedParameter(param);
	int len = namedParam->GetNamedParamStringLength(_T("PARENTS"));
	if (len == 0) {
		return true;
	}

	CString parents;
	namedParam->GetNamedParamString(_T("PARENTS"), parents.GetBuffer(len), len);
	parents.ReleaseBuffer();

	int depth = 0;
	int n = 0;
	CString token = parents.Tokenize(_T("/"), n);
	while(token.IsEmpty() == FALSE) {

		if (depth >= 8) {
			// 深さは8まで
			if (errMsg) {
				*errMsg = "コマンドの階層が上限(8階層)に達しました";
			}
			return false;
		}
		if (token == in->mParentCommandName) {
			// 呼び出し元に自分自身がいる(循環参照)
			if (errMsg) {
				*errMsg = "コマンドの循環参照を検知しました";
			}
			return false;
		}
		token = parents.Tokenize(_T("/"), n);
		depth++;
	}

	return true;
}

}}}

