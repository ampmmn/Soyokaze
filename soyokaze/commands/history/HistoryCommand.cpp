#include "pch.h"
#include "HistoryCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace history {

using CommandRepository = soyokaze::core::CommandRepository;
using Command = soyokaze::core::Command;

struct HistoryCommand::PImpl
{
	CString mKeyword;
	Parameter mParam;
	Command* mCmd = nullptr;
};


HistoryCommand::HistoryCommand(const CString& keyword) : in(std::make_unique<PImpl>())
{
	this->mName = keyword;

	in->mKeyword = keyword;
	in->mParam.SetWholeString(keyword);

	auto cmdRepo = CommandRepository::GetInstance();
	auto cmd = cmdRepo->QueryAsWholeMatch(in->mParam.GetCommandString(), true);
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

BOOL HistoryCommand::Execute(const Parameter& param)
{
	if (in->mCmd == nullptr) {
		return FALSE;
	}

	Parameter paramTmp(in->mParam);
	param.CopyNamedParamTo(paramTmp);

	return in->mCmd->Execute(paramTmp);
}

HICON HistoryCommand::GetIcon()
{
	if (in->mCmd) {
		return in->mCmd->GetIcon();
	}
	HICON h =IconLoader::Get()->LoadUnknownIcon();
	return h;
}

soyokaze::core::Command*
HistoryCommand::Clone()
{
	return new HistoryCommand(in->mKeyword);
}

} // end of namespace history
} // end of namespace commands
} // end of namespace soyokaze

