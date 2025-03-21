#include "pch.h"
#include "FilterAdhocCommand.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/core/CommandParameter.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/core/CommandRepository.h"
#include "icon/IconLoader.h"
#include "utility/RefPtr.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ShellExecCommand = launcherapp::commands::shellexecute::ShellExecCommand;

using CommandRepository = launcherapp::core::CommandRepository;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;


namespace launcherapp {
namespace commands {
namespace filter {

struct FilterAdhocCommand::PImpl
{
	CommandParam mParam;
	FilterResult mResult;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(FilterAdhocCommand)

FilterAdhocCommand::FilterAdhocCommand(
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

FilterAdhocCommand::~FilterAdhocCommand()
{
}

CString FilterAdhocCommand::GetName()
{
	return in->mParam.mName + _T(" ") + mName;
}

CString FilterAdhocCommand::GetDescription()
{
	CString str;
	str.Format(_T("%s"), (LPCTSTR)in->mResult.mDisplayName);
	return str;

}

CString FilterAdhocCommand::GetGuideString()
{
	return _T("Enter:開く");
}

CString FilterAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_FILTERCOMMAND);
	return TEXT_TYPE;
}

BOOL FilterAdhocCommand::Execute(Parameter* param)
{
	CString argSub = in->mParam.mAfterCommandParam;
	argSub.Replace(_T("$select"), in->mResult.mDisplayName);
	ExpandMacros(argSub);

	auto namedParam = GetCommandNamedParameter(param);

	CString parents;
	int len = namedParam->GetNamedParamStringLength(_T("PARENTS"));
	if (len > 0) {
		namedParam->GetNamedParamString(_T("PARENTS"), parents.GetBuffer(len), len);
		parents.ReleaseBuffer();
	}

	// 呼び出し元に自分自身を追加
	if (parents.IsEmpty() == FALSE) {
		parents += _T("/");
	}
	parents += in->mParam.mName;

	int type = in->mParam.mPostFilterType;
	if (type == POSTFILTER_COMMAND) {

		RefPtr<CommandParameterBuilder> paramSub(CommandParameterBuilder::Create(), false);
		paramSub->AddArgument(argSub);
		paramSub->SetNamedParamString(_T("PARENTS"), parents);

		// Ctrlキーが押されているかを設定
		if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
			paramSub->SetNamedParamBool(_T("CtrlKeyPressed"), true);
		}
		if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
			paramSub->SetNamedParamBool(_T("ShiftKeyPressed"), true);
		}
		if (GetAsyncKeyState(VK_LWIN) & 0x8000) {
			paramSub->SetNamedParamBool(_T("WinKeyPressed"), true);
		}
		if (GetAsyncKeyState(VK_MENU) & 0x8000) {
			paramSub->SetNamedParamBool(_T("AltKeyPressed"), true);
		}

		// 他のコマンドを実行
		auto cmdRepo = CommandRepository::GetInstance();
		RefPtr<launcherapp::core::Command> command(cmdRepo->QueryAsWholeMatch(in->mParam.mAfterCommandName, false));
		if (command) {
			command->Execute(paramSub);
		}
		return true;
	}

	if (type == POSTFILTER_SUBPROCESS) {
		// 他のファイルを実行/URLを開く
		ShellExecCommand cmd;

		CString path = in->mParam.mAfterFilePath;
		path.Replace(_T("$select"), in->mResult.mDisplayName);
		ExpandMacros(path);
		cmd.SetPath(path);

		cmd.SetArgument(argSub);

		path = in->mParam.mAfterDir;
		path.Replace(_T("$select"), in->mResult.mDisplayName);
		ExpandMacros(path);
		cmd.SetWorkDir(path);
		cmd.SetShowType(in->mParam.GetAfterShowType());

		return cmd.Execute(CommandParameterBuilder::EmptyParam());
	}

	if (type == POSTFILTER_CLIPBOARD) {
		// クリップボードにコピー
		Clipboard::Copy(argSub);
		return true;
	}
	return true;
}

HICON FilterAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadPromptIcon();
}

launcherapp::core::Command*
FilterAdhocCommand::Clone()
{
	return new FilterAdhocCommand(in->mParam, in->mResult);
}

CString FilterAdhocCommand::GetSourceName()
{
	return in->mParam.mName;
}

bool FilterAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_EXTRACANDIDATE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidate*)this;
		return true;
	}
	return false;
}



} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


