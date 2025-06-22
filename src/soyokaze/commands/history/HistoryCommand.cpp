#include "pch.h"
#include "HistoryCommand.h"
#include "commands/history/HistoryCommandQueryRequest.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "commands/common/CommandParameterFunctions.h"
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
	Command* GetCommand() {

		if (mCmd) {
			return mCmd;
		}

		// 履歴キーワードによる絞り込みを実施
		auto req = new CommandQueryRequest(mKeyword);
		CommandRepository::GetInstance()->Query(req);

		// 検索完了を待つ
		if (req->WaitComplete(2000) == false) {
			spdlog::error(_T("HISTORY: query timeout occurred. keyword:{}"), (LPCTSTR)mKeyword);
			req->Release();
			return nullptr;
		}

		BOOL result = TRUE;

		std::vector<launcherapp::core::Command*> items;

		// 結果を取得し、先頭の候補を実行する
		if (req->GetResult(items) && items.empty() == false) {

			// 履歴コマンドは除外
			auto thisTypeName = mThisPtr->GetTypeDisplayName();
			for (auto cmd : items) {
				if (cmd->GetTypeDisplayName() == thisTypeName) {
					continue;
				}
				mCmd = cmd;
				mCmd->AddRef();
				break;
			}

			for (auto cmd : items) {
				cmd->Release();
			}
		}

		req->Release();

		return mCmd;
	}

	Command* mThisPtr{nullptr};
	CString mKeyword;
	Command* mCmd{nullptr};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(HistoryCommand)

HistoryCommand::HistoryCommand(const CString& keyword) : in(std::make_unique<PImpl>())
{
	this->mName = keyword;

	in->mThisPtr = this;
	in->mKeyword = keyword;
}

HistoryCommand::~HistoryCommand()
{
	if (in->mCmd) {
		in->mCmd->Release();
	}
}

CString HistoryCommand::GetDescription()
{
	auto cmd = in->GetCommand();
	if (cmd == nullptr) {
		return GetName();
	}
	return cmd->GetDescription();
}

CString HistoryCommand::GetGuideString()
{
	auto cmd = in->GetCommand();
	if (cmd == nullptr) {
		return _T("⏎:開く");
	}

	return cmd->GetGuideString();
}

CString HistoryCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_HISTORY);
	return TEXT_TYPE;
}

BOOL HistoryCommand::Execute(Parameter* param)
{
	auto cmd = in->GetCommand();

	BOOL result = TRUE;
	if (cmd) {
		auto builder = CommandParameterBuilder::Create(in->mKeyword);
		bool hasParameter = builder->HasParameter();
		builder->Release();

		RefPtr<Parameter> paramTmp(param->Clone(), false);
		if (hasParameter) {
			// 履歴がパラメータを持つ場合は、履歴の方を優先する
			paramTmp->SetWholeString(in->mKeyword);
		}

		auto namedParam = launcherapp::commands::common::GetCommandNamedParameter(paramTmp);
		namedParam->SetNamedParamBool(_T("RunAsHistory"), true);

		result = cmd->Execute(paramTmp);
	}
	return result;
}

HICON HistoryCommand::GetIcon()
{
	return IconLoader::Get()->LoadHistoryIcon();
}

launcherapp::core::Command*
HistoryCommand::Clone()
{
	return new HistoryCommand(in->mKeyword);
}

} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

