#include "pch.h"
#include "DirectoryIndexAdhocCommand.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommandParam.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/core/CommandRepository.h"
#include "SharedHwnd.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;


namespace launcherapp {
namespace commands {
namespace url_directoryindex {

struct DirectoryIndexAdhocCommand::PImpl
{
	bool OpenURL(const CString& url);

	URLDirectoryIndexCommand* mBaseCmd = nullptr;
	QueryResult mResult;
};

bool DirectoryIndexAdhocCommand::PImpl::OpenURL(const CString& url)
{
	Parameter param;
	SubProcess exec(param);
	SubProcess::ProcessPtr process;
	exec.Run(url, param.GetParameterString(), process);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

DirectoryIndexAdhocCommand::DirectoryIndexAdhocCommand(
	URLDirectoryIndexCommand* baseCmd,
 	const QueryResult& result
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mBaseCmd = baseCmd;
	baseCmd->AddRef();

	in->mResult = result;
}

DirectoryIndexAdhocCommand::~DirectoryIndexAdhocCommand()
{
	if (in->mBaseCmd) {
		in->mBaseCmd->Release();
	}
}

CString DirectoryIndexAdhocCommand::GetName()
{
	CommandParam param;
	in->mBaseCmd->GetParam(param);
	return param.mName + _T(" ") + PathFindFileName(in->mResult.mLinkText);
}

CString DirectoryIndexAdhocCommand::GetDescription()
{
	CommandParam param;
	in->mBaseCmd->GetParam(param);

	CString url = param.CombineURL(in->mResult.mLinkPath);

	CString str;
	str.Format(_T("%s"), (LPCTSTR)url);
	return str;

}

CString DirectoryIndexAdhocCommand::GetGuideString()
{
	return _T("Enter:開く");
}

CString DirectoryIndexAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_FILTERCOMMAND);
	return TEXT_TYPE;
}

BOOL DirectoryIndexAdhocCommand::Execute(const Parameter& param_)
{
	CommandParam param;
	in->mBaseCmd->GetParam(param);
	CString url = param.CombineURL(in->mResult.mLinkPath);
	if (param.IsContentURL(url)) {
		return in->OpenURL(url) ? TRUE : FALSE;
	}

	in->mBaseCmd->SetSubPath(in->mResult.mLinkPath);

	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 2, 1, 0);

	auto cmdline = in->mBaseCmd->GetName();
	cmdline += _T(" ");
	SendMessage(sharedWnd.GetHwnd(), WM_APP+11, 0, (LPARAM)(LPCTSTR)cmdline);

	in->mBaseCmd->LoadCanidates();
	return TRUE;
}

HICON DirectoryIndexAdhocCommand::GetIcon()
{
	return IconLoader::Get()->LoadPromptIcon();
}

launcherapp::core::Command*
DirectoryIndexAdhocCommand::Clone()
{
	return new DirectoryIndexAdhocCommand(in->mBaseCmd, in->mResult);
}



} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp


