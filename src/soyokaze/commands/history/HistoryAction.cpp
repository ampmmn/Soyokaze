#include "pch.h"
#include "HistoryAction.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/core/ActionParameter.h"

using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;


namespace launcherapp { namespace commands { namespace history {

HistoryAction::HistoryAction(
	launcherapp::actions::core::Action* realAction,
	const CString& keyword
) : mRealAction(realAction), mKeyword(keyword)
{
	if (realAction) {
		realAction->AddRef();
	}
}

HistoryAction::~HistoryAction()
{
	if (mRealAction) {
		mRealAction->Release();
	}
}

// アクションの内容を示す名称
CString HistoryAction::GetDisplayName()
{
	return mRealAction->GetDisplayName();
}

// アクションを実行する
bool HistoryAction::Perform(Parameter* param, String* errMsg)
{
	auto builder = ParameterBuilder::Create(mKeyword);
	bool hasParameter = builder->HasParameter();
	builder->Release();

	RefPtr<Parameter> paramTmp(param->Clone(), false);
	if (hasParameter) {
		// 履歴がパラメータを持つ場合は、履歴の方を優先する
		paramTmp->SetWholeString(mKeyword);
	}

	auto namedParam = launcherapp::commands::common::GetNamedParameter(paramTmp);
	namedParam->SetNamedParamBool(_T("RunAsHistory"), true);
	return mRealAction->Perform(paramTmp, errMsg);
}

// ガイド欄などに表示するかどうか
bool HistoryAction::IsVisible()
{
	return mRealAction->IsVisible();
}

}}} // end of namespace launcherapp::commands::history

