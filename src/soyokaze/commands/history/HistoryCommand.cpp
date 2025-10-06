#include "pch.h"
#include "HistoryCommand.h"
#include "commands/history/HistoryAction.h"
#include "core/IFIDDefine.h"
#include "commands/history/HistoryCommandQueryRequest.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/core/ActionParameter.h"
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
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;
using CommandQueryResult = launcherapp::commands::core::CommandQueryResult;
using ContextMenuSource = launcherapp::commands::core::ContextMenuSource;

constexpr LPCTSTR TYPENAME = _T("HistoryCommand");

struct HistoryCommand::PImpl
{
	Command* GetCommand() {

		if (mCmd) {
			return mCmd;
		}
		if (mIsCommandUnavailable) {
			// コマンドが見つからなかった場合は以後は探さない
			return nullptr;
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

		CommandQueryResult* items = nullptr;

		// 結果を取得し、先頭の候補を実行する
		if (req->GetResult(&items) && items->IsEmpty() == false) {

			// 履歴コマンドは除外
			size_t count = items->GetCount();
			auto thisTypeName = mThisPtr->GetTypeDisplayName();
			for (size_t i = 0; i < count; ++i) {
				auto cmd = items->GetItem(i);
				if (cmd->GetTypeDisplayName() == thisTypeName) {
					cmd->Release();
					mIsCommandUnavailable = true;
					continue;
				}
				mCmd = cmd;
				// mCmd->AddRef();   / GetItemによりコマンドの参照カウントが+1されるため、ここでは不要
				break;
			}
		}

		if (items) {
			items->Release();
		}
		req->Release();

		if (mCmd == nullptr) {
			mIsCommandUnavailable = true;
		}


		return mCmd;
	}

	Command* mThisPtr{nullptr};
	CString mKeyword;
	Command* mCmd{nullptr};
	bool mIsCommandUnavailable{false};
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

CString HistoryCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool HistoryCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	auto cmd = in->GetCommand();
	if (cmd == nullptr) {
		return false;
	}

	RefPtr<Action> realAction;
	if (cmd->GetAction(modifierFlags, &realAction) == false) {
		return false;
	}

	*action = new HistoryAction(realAction.get(), in->mKeyword);
	return true;
}

HICON HistoryCommand::GetIcon()
{
	auto cmd = in->GetCommand();
	if (cmd == nullptr) {
		return IconLoader::Get()->LoadHistoryIcon();
	}
	return cmd->GetIcon();
}

launcherapp::core::Command*
HistoryCommand::Clone()
{
	return new HistoryCommand(in->mKeyword);
}

CString HistoryCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_HISTORY);
	return TEXT_TYPE;
}

// メニューの項目数を取得する
int HistoryCommand::GetMenuItemCount()
{
	auto cmd = in->GetCommand();
	if (cmd == nullptr) {
		return 0;
	}

	RefPtr<ContextMenuSource> menuSrc;
	if (cmd->QueryInterface(IFID_CONTEXTMENUSOURCE, (void**)&menuSrc) == false) {
		return 0;
	}
	return menuSrc->GetMenuItemCount();
}

// メニューに対応するアクションを取得する
bool HistoryCommand::GetMenuItem(int index, Action** action)
{
	auto cmd = in->GetCommand();
	if (cmd == nullptr) {
		return false;
	}

	RefPtr<ContextMenuSource> menuSrc;
	if (cmd->QueryInterface(IFID_CONTEXTMENUSOURCE, (void**)&menuSrc) == false) {
		return false;
	}
	return menuSrc->GetMenuItem(index, action);
}

bool HistoryCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (ContextMenuSource*)this;
		return true;
	}
	return false;
}

} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

