#include "pch.h"
#include "framework.h"
#include "WebSearchAdhocCommand.h"
#include "commands/websearch/WebSearchCommand.h"
#include "commands/websearch/WebSearchAdhocCommand.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/SubProcess.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace websearch {

struct WebSearchAdhocCommand::PImpl
{
	WebSearchCommand* mBaseCommand = nullptr;
	CString mURL;
	CString mErrorMsg;
};


WebSearchAdhocCommand::WebSearchAdhocCommand(
	WebSearchCommand* baseCommand,
 	const CString& displayName,
 	const CString& showUrl
) : 
	AdhocCommandBase(displayName, baseCommand->GetDescription()),
	in(std::make_unique<PImpl>())
{
	in->mBaseCommand = baseCommand;
	baseCommand->AddRef();
	in->mURL = showUrl;
}

WebSearchAdhocCommand::~WebSearchAdhocCommand()
{
	if (in->mBaseCommand) {
		in->mBaseCommand->Release();
	}
}

CString WebSearchAdhocCommand::GetGuideString()
{
	return _T("Enter:検索を実行");
}

CString WebSearchAdhocCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
	return TEXT_TYPE;
}

BOOL WebSearchAdhocCommand::Execute(const Parameter& param)
{
	SubProcess exec(param);

	SubProcess::ProcessPtr process;
	if (exec.Run(in->mURL, process) == FALSE) {
		in->mErrorMsg = process->GetErrorMessage();
		return FALSE;
	}
	return TRUE;
}

HICON WebSearchAdhocCommand::GetIcon()
{
	return in->mBaseCommand->GetIcon();
}

launcherapp::core::Command*
WebSearchAdhocCommand::Clone()
{
	return new WebSearchAdhocCommand(in->mBaseCommand, GetName(), in->mURL);
}

} // end of namespace websearch
} // end of namespace commands
} // end of namespace launcherapp

