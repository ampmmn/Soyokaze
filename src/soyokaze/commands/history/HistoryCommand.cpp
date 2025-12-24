#include "pch.h"
#include "HistoryCommand.h"
#include "commands/history/HistoryAction.h"
#include "commands/history/HistoryCommandResolver.h"
#include "core/IFIDDefine.h"
#include "actions/core/ActionParameter.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace history {

using Command = launcherapp::core::Command;
using ContextMenuSource = launcherapp::commands::core::ContextMenuSource;
using SelectionBehavior = launcherapp::core::SelectionBehavior;

constexpr LPCTSTR TYPENAME = _T("HistoryCommand");

struct HistoryCommand::PImpl
{
	Command* GetCommand() {

		if (mCmd) {
			return mCmd;
		}

		auto resolver = HistoryCommandResolver::GetInstance();
		resolver->Resolve(mKeyword, &mCmd);

		return mCmd;
	}

	CString mKeyword;
	Command* mCmd{nullptr};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(HistoryCommand)

HistoryCommand::HistoryCommand(const CString& keyword) : in(std::make_unique<PImpl>())
{
	this->mName = keyword;

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

// 選択された
void HistoryCommand::OnSelect(Command*)
{
	// 何もしない
}

// 選択解除された
void HistoryCommand::OnUnselect(Command*) 
{
	// 何もしない
}

// 実行後のウインドウを閉じる方法
SelectionBehavior::CloseWindowPolicy
HistoryCommand::GetCloseWindowPolicy(uint32_t modifierMask)
{
	return SelectionBehavior::CLOSEWINDOW_ASYNC;
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool HistoryCommand::CompleteKeyword(CString& keyword, int& startPos, int& endPos)
{
	if (in->mKeyword.GetLength() < 3) {
		return false;
	}
	bool isLocalPath = in->mKeyword[1] == _T(':') && in->mKeyword[2] == _T('\\');
	bool isUNCPath = in->mKeyword[0] == _T('\\') && in->mKeyword[1] == _T('\\');
	if (isLocalPath == false && isUNCPath == false) {
		return false;
	}

	// 履歴キーワードが絶対パスを示すものだった場合、パスを補完する
	// (既定の補完動作である末尾にスペースを入れないようにする)
	keyword = in->mKeyword;
	startPos = keyword.GetLength();
	endPos = keyword.GetLength();

	return true;
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
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
	return false;
}

} // end of namespace history
} // end of namespace commands
} // end of namespace launcherapp

