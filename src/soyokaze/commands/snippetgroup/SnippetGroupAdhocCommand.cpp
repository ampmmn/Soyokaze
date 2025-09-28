#include "pch.h"
#include "framework.h"
#include "SnippetGroupAdhocCommand.h"
#include "commands/snippetgroup/SnippetGroupParam.h"
#include "commands/common/ExpandFunctions.h"
#include "actions/clipboard/CopyClipboardAction.h"
#include "icon/IconLoader.h"

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
	SnippetGroupParam mParam;
	Item mItem;
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(SnippetGroupAdhocCommand)

SnippetGroupAdhocCommand::SnippetGroupAdhocCommand(
	const SnippetGroupParam& param,
	const Item& item
) : 
	AdhocCommandBase(_T(""), _T("")),
	in(std::make_unique<PImpl>())
{
	in->mParam = param;
	in->mItem = item;
}

SnippetGroupAdhocCommand::~SnippetGroupAdhocCommand()
{
}

CString SnippetGroupAdhocCommand::GetName()
{
	return in->mParam.mName + _T(" ") + in->mItem.mName;
}

CString SnippetGroupAdhocCommand::GetDescription()
{
	return in->mItem.mDescription;

}

CString SnippetGroupAdhocCommand::GetTypeDisplayName()
{
	return _T("定型文グループ");
}

bool SnippetGroupAdhocCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
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
	return IconLoader::Get()->GetImageResIcon(-5301);
}

launcherapp::core::Command*
SnippetGroupAdhocCommand::Clone()
{
	return new SnippetGroupAdhocCommand(in->mParam, in->mItem);
}

CString SnippetGroupAdhocCommand::GetSourceName()
{
	return in->mParam.mName;
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

