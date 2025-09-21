#include "pch.h"
#include "FilterAdhocCommand.h"
#include "actions/core/ActionParameter.h"
#include "commands/filter/FilterCommandParam.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/core/CommandRepository.h"
#include "actions/builtin/RunCommandAction.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/clipboard/CopyClipboardAction.h"
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
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;

using RunCommandAction = launcherapp::actions::builtin::RunCommandAction;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

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
	return _T("⏎:開く");
}

CString FilterAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_FILTERCOMMAND);
	return TEXT_TYPE;
}

bool FilterAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	int type = in->mParam.mPostFilterType;
	if (type == POSTFILTER_COMMAND) {
		return CreatePostFilterCommandAction(modifierFlags, action);
	}
	else if (type == POSTFILTER_SUBPROCESS) {
		return CreatePostFilterSubprocessAction(modifierFlags, action);
	}
	else if (type == POSTFILTER_CLIPBOARD) {
		return CreatePostFilterCopyAction(modifierFlags, action);
	}
	return false;
}

bool FilterAdhocCommand::CreatePostFilterCommandAction(uint32_t modifierFlags, Action** action)
{
	auto a = new RunCommandAction(in->mParam.mName, in->mParam.mAfterCommandName, modifierFlags);
	*action = a;
	return true;
}

bool FilterAdhocCommand::CreatePostFilterSubprocessAction(uint32_t modifierFlags, Action** action)
{
	// コマンド実行時に起動するファイルパス上の"$select"を選択値に置換
	CString path = in->mParam.mAfterFilePath;
	path.Replace(_T("$select"), in->mResult.mDisplayName);
	ExpandMacros(path);

	// コマンドパラメータ上の"$select"を選択値に置換
	CString param = in->mParam.mAfterCommandParam;
	param.Replace(_T("$select"), in->mResult.mDisplayName);
	ExpandMacros(param);

	// コマンド実行時のカレントディレクトリも同様に置換
	CString workDir = in->mParam.mAfterDir;
	workDir.Replace(_T("$select"), in->mResult.mDisplayName);
	ExpandMacros(workDir);

	// 他のファイルを実行/URLを開く
	*action = new ExecuteAction(path, param, workDir, in->mParam.GetAfterShowType());
	return true;
}

bool FilterAdhocCommand::CreatePostFilterCopyAction(uint32_t modifierFlags, Action** action)
{
	// コマンドパラメータに基づき、選択値を置換
	CString param = in->mParam.mAfterCommandParam;
	param.Replace(_T("$select"), in->mResult.mDisplayName);
	ExpandMacros(param);

	// 置換した値をクリップボードにコピー
	*action = new CopyTextAction(param);

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


