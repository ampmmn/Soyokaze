#include "pch.h"
#include "PostFilterCommand.h"
#include "actions/core/ActionParameter.h"
#include "actions/core/ActionBase.h"
#include "actions/builtin/ExecuteAction.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "utility/RefPtr.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;

using Action = launcherapp::actions::core::Action;
using ActionBase = launcherapp::actions::core::ActionBase;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction; 

namespace launcherapp { namespace commands { namespace filter {

class PostFilterAction : virtual public ActionBase
{
public:
	PostFilterAction(Action* realAction, const CString& name, const CString& args) :
	 	mRealAction(realAction, true), mDisplayName(name), mArguments(args)
	{
	}
	~PostFilterAction() 
	{
	}

	// アクションの内容を示す名称
	CString GetDisplayName() override {
		ASSERT(mRealAction.get());
		return mRealAction->GetDisplayName();
	}
	// アクションを実行する
	bool Perform(Parameter* param, String* errMsg) override
	{
		CString argSub = mArguments;
		argSub.Replace(_T("$select"), mDisplayName);
		ExpandMacros(argSub);

		RefPtr<Parameter> paramSub(param->Clone(), false);
		paramSub->SetParameterString(argSub);

		RefPtr<ExecuteAction> execAction;
		if (mRealAction->QueryInterface(IFID_EXECUTEACTION, (void**)&execAction)) {
			// フィルタコマンド経由での実行のときは履歴登録しない
			execAction->SetHistoryPolicy(ExecuteAction::HISTORY_NONE);
		}

		return mRealAction->Perform(paramSub.get(), errMsg);
	}
	// ガイド欄などに表示するかどうか
	bool IsVisible() override {
		return mRealAction->IsVisible();
	}

	RefPtr<Action> mRealAction;
	CString mDisplayName;
	CString mArguments;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct PostFilterCommand::PImpl
{
	launcherapp::core::Command* GetRealCommand() {
		if (mRealCommand.get()) {
			return mRealCommand.get();
		}
		// 実行対象のコマンドを取得
		auto cmdRepo = CommandRepository::GetInstance();
		mRealCommand.reset(cmdRepo->QueryAsWholeMatch(mParam.mAfterCommandName, false));
		return mRealCommand.get();
	}
	RefPtr<launcherapp::core::Command> mRealCommand;
	CommandParam mParam;
	FilterResult mResult;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(PostFilterCommand)

PostFilterCommand::PostFilterCommand(
	const CommandParam& param,
 	const FilterResult& result
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	param.ReplaceCandidateText(result.mDisplayName, mName);

	in->mParam = param;
	in->mResult = result;
}

PostFilterCommand::~PostFilterCommand()
{
}

CString PostFilterCommand::GetName()
{
	return in->mParam.mName + _T(" ") + mName;
}

CString PostFilterCommand::GetDescription()
{
	CString str;
	str.Format(_T("%s"), (LPCTSTR)in->mResult.mDisplayName);
	return str;

}

CString PostFilterCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_FILTERCOMMAND);
	return TEXT_TYPE;
}

bool PostFilterCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto realCommand = in->GetRealCommand();
	if (realCommand == nullptr) {
		return false;
	}

	RefPtr<Action> realAction;
	if (realCommand->GetAction(hotkeyAttr, &realAction) == false) {
		return false;
	}

	*action = new PostFilterAction(realAction.get(), in->mResult.mDisplayName, in->mParam.mAfterCommandParam);
	return true;
}

HICON PostFilterCommand::GetIcon()
{
	return in->GetRealCommand()->GetIcon();
}

launcherapp::core::Command*
PostFilterCommand::Clone()
{
	return new PostFilterCommand(in->mParam, in->mResult);
}


}}}


