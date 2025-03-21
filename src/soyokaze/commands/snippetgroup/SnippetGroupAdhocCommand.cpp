#include "pch.h"
#include "framework.h"
#include "SnippetGroupAdhocCommand.h"
#include "commands/snippetgroup/SnippetGroupParam.h"
#include "commands/common/SubProcess.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "commands/common/CommandParameterFunctions.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using SubProcess = launcherapp::commands::common::SubProcess;

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

CString SnippetGroupAdhocCommand::GetGuideString()
{
	CString guideStr(_T("Enter:コピー"));
	return guideStr;
}

CString SnippetGroupAdhocCommand::GetTypeDisplayName()
{
	return _T("定型文グループ");
}

BOOL SnippetGroupAdhocCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// 値をコピー
	auto value = in->mItem.mText;
	ExpandMacros(value);
	Clipboard::Copy(value);
	return TRUE;
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

