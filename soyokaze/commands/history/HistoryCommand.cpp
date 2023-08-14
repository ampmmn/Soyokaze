#include "pch.h"
#include "HistoryCommand.h"
#include "core/CommandRepository.h"
#include "IconLoader.h"
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
	Command* mCmd;
	uint32_t mRefCount;
};


HistoryCommand::HistoryCommand(const CString& keyword) : in(new PImpl)
{
	in->mRefCount = 1;
	in->mKeyword = keyword;
	in->mParam.SetWholeString(keyword);
	in->mCmd = nullptr;

	auto cmdRepo = CommandRepository::GetInstance();
	auto cmd = cmdRepo->QueryAsWholeMatch(in->mParam.GetCommandString(), false);
	if (cmd == nullptr) {
		return;
	}

	in->mCmd = cmd;
}

HistoryCommand::~HistoryCommand()
{
	if (in->mCmd) {
		in->mCmd->Release();
	}
}

CString HistoryCommand::GetName()
{
	return in->mKeyword;
}

CString HistoryCommand::GetDescription()
{
	if (in->mCmd) {
		return in->mCmd->GetDescription();
	}
	return _T("");
}

CString HistoryCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_HISTORY);
	return TEXT_TYPE;
}

BOOL HistoryCommand::Execute()
{
	return Execute(in->mParam);
}

BOOL HistoryCommand::Execute(const Parameter& param)
{
	param;

	if (in->mCmd == nullptr) {
		return FALSE;
	}
	return in->mCmd->Execute(in->mParam);
}

CString HistoryCommand::GetErrorString()
{
	return _T("");
}

HICON HistoryCommand::GetIcon()
{
	if (in->mCmd) {
		return in->mCmd->GetIcon();
	}
	HICON h =IconLoader::Get()->LoadUnknownIcon();
	return h;
}

int HistoryCommand::Match(Pattern* pattern)
{
	return Pattern::Mismatch;
}

bool HistoryCommand::IsEditable()
{
	return false;
}

int HistoryCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
HistoryCommand::Clone()
{
	return new HistoryCommand(in->mKeyword);
}

bool HistoryCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t HistoryCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t HistoryCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace mailto
} // end of namespace commands
} // end of namespace soyokaze

