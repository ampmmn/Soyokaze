#include "pch.h"
#include "framework.h"
#include "OneNoteCommand.h"
#include "commands/onenote/OneNoteCommandParam.h"
#include "commands/onenote/OneNoteAppProxy.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace onenote {

struct OneNoteCommand::PImpl
{
	CString mDispName;
	CString mNotebookName;
	CStringW mNavigateID;
};




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(OneNoteCommand)

OneNoteCommand::OneNoteCommand() : in(new PImpl)
{
}


OneNoteCommand::OneNoteCommand(
	CommandParam* param,
	const OneNoteBook& book,
	const OneNoteSection& section,
	const OneNotePage& page
) : in(new PImpl)
{
	in->mNavigateID = page.GetID();
	in->mNotebookName = book.GetNickName();

	auto& dispName = in->mDispName;
	if (param->mPrefix.IsEmpty() == FALSE) {
		dispName = param->mPrefix;
		dispName += _T(" ");
	}
	dispName += section.GetName();
	dispName += _T("/");
	dispName += page.GetName();
}

OneNoteCommand::~OneNoteCommand()
{
}

CString OneNoteCommand::GetName()
{
	return in->mDispName;
}

CString OneNoteCommand::GetDescription()
{
	return in->mNotebookName;
}

CString OneNoteCommand::GetGuideString()
{
	return _T("⏎:ページをOneNoteで開く");
}

CString OneNoteCommand::GetTypeDisplayName()
{
	return TypeDisplayName() + _T("(") + in->mNotebookName + _T(")");
}

BOOL OneNoteCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	OneNoteAppProxy app;
	if (app.NavigateTo(in->mNavigateID) == false) {
		spdlog::error(L"Failed to NavigateTo id:{}", (LPCWSTR)in->mNavigateID);
	}
	return TRUE;
}

CString OneNoteCommand::GetErrorString()
{
	return _T("");
}

HICON OneNoteCommand::GetIcon()
{
	return IconLoader::Get()->LoadExtensionIcon(_T(".one"));
}

launcherapp::core::Command*
OneNoteCommand::Clone()
{
	auto newCmd = new OneNoteCommand();
	newCmd->in->mDispName = in->mDispName;
	newCmd->in->mNotebookName = in->mNotebookName;
	newCmd->in->mNavigateID = in->mNavigateID;

	return newCmd;
}

CString OneNoteCommand::TypeDisplayName()
{
	return _T("OneNote");
}

}}} // end of namespace launcherapp::commands::onenote
