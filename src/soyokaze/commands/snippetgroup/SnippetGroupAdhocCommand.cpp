#include "pch.h"
#include "framework.h"
#include "SnippetGroupAdhocCommand.h"
#include "commands/snippetgroup/SnippetGroupCommand.h"
#include "commands/snippetgroup/SnippetGroupParam.h"
#include "commands/common/ExpandFunctions.h"
#include "actions/clipboard/CopyClipboardAction.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using CopyTextAction = launcherapp::actions::clipboard::CopyTextAction;

namespace launcherapp {
namespace commands {
namespace snippetgroup {

struct SnippetGroupAdhocCommand::PImpl
{
	SnippetGroupCommand* mBaseCommand{nullptr};
	Item mItem;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(SnippetGroupAdhocCommand)

SnippetGroupAdhocCommand::SnippetGroupAdhocCommand(
	SnippetGroupCommand* baseCommand,
	const Item& item
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mBaseCommand = baseCommand;
	in->mItem = item;
}

SnippetGroupAdhocCommand::~SnippetGroupAdhocCommand()
{
}

CString SnippetGroupAdhocCommand::GetName()
{
	return in->mBaseCommand->GetParam().mName + _T(" ") + in->mItem.mName;
}

CString SnippetGroupAdhocCommand::GetDescription()
{
	return in->mItem.mDescription;

}

CString SnippetGroupAdhocCommand::GetTypeDisplayName()
{
	return _T("定型文グループ");
}

bool SnippetGroupAdhocCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	if (hotkeyAttr.GetModifiers() == 0) {
		// 値をコピー
		auto value = in->mItem.mText;
		ExpandMacros(value);
		*action = new CopyTextAction(value);
		return true;
	}
	return false;
}

HICON SnippetGroupAdhocCommand::GetIcon()
{
	return in->mBaseCommand->GetIcon();
}

launcherapp::core::Command*
SnippetGroupAdhocCommand::Clone()
{
	return new SnippetGroupAdhocCommand(in->mBaseCommand, in->mItem);
}

CString SnippetGroupAdhocCommand::GetSourceName()
{
	return in->mBaseCommand->GetParam().mName;
}

bool SnippetGroupAdhocCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
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


} // end of namespace snippetgroup
} // end of namespace commands
} // end of namespace launcherapp

