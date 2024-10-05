#include "pch.h"
#include "HistoryCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace history {

using CommandRepository = launcherapp::core::CommandRepository;
using Command = launcherapp::core::Command;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;

constexpr LPCTSTR TYPENAME = _T("HistoryCommand");

struct HistoryCommand::PImpl
{
	CString mKeyword;
	Command* mCmd = nullptr;
};


HistoryCommand::HistoryCommand(const CString& keyword) : in(std::make_unique<PImpl>())
{
	this->mName = keyword;

	in->mKeyword = keyword;

	auto paramTmp = CommandParameterBuilder::Create(keyword);
	auto commandPart = paramTmp->GetCommandString();
	paramTmp->Release();
	

	auto cmdRepo = CommandRepository::GetInstance();
	auto cmd = cmdRepo->QueryAsWholeMatch(commandPart, true);
	if (cmd == nullptr) {
		return;
	}

	in->mCmd = cmd;

	this->mDescription = cmd->GetDescription();
}

HistoryCommand::~HistoryCommand()
{
	if (in->mCmd) {
		in->mCmd->Release();
	}
}

CString HistoryCommand::GetGuideString()
{
	if (in->mCmd == nullptr) {
		return _T("Enter:開く");
	}

	return in->mCmd->GetGuideString();
}

CString HistoryCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_HISTORY);
	return TEXT_TYPE;
}

BOOL HistoryCommand::Execute(Parameter* param)
{
	if (in->mCmd == nullptr) {
		return FALSE;
	}

	auto paramTmp = param->Clone();
	paramTmp->SetWholeString(in->mKeyword);

	BOOL result = in->mCmd->Execute(paramTmp);

	paramTmp->Release();

	return result;
}

HICON HistoryCommand::GetIcon()
{
	if (in->mCmd) {
		return in->mCmd->GetIcon();
	}
	HICON h =IconLoader::Get()->LoadUnknownIcon();
	return h;
}

launcherapp::core::Command*
HistoryCommand::Clone()
{
	return new HistoryCommand(in->mKeyword);
}

} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

