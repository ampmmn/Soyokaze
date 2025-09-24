#include "pch.h"
#include "framework.h"
#include "WorksheetCommand.h"
#include "commands/activate_worksheet/ExcelWorksheets.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::builtin;

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

struct WorksheetCommand::PImpl
{
	Worksheet* mWorksheet{nullptr};
};


IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(WorksheetCommand)

WorksheetCommand::WorksheetCommand(
	Worksheet* sheet
) : in(std::make_unique<PImpl>())
{
	in->mWorksheet = sheet;
	sheet->AddRef();

	this->mName = fmt::format(_T("{} ({})"), sheet->GetSheetName(), sheet->GetWorkbookName()).c_str();
	this->mDescription = sheet->GetSheetName().c_str();

}

WorksheetCommand::~WorksheetCommand()
{
	if (in->mWorksheet) {
		in->mWorksheet->Release();
	}
}

CString WorksheetCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool WorksheetCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		*action = new CallbackAction(_T("表示"), [&](Parameter*, String*) -> bool {
			return in->mWorksheet->Activate(false);
		});
		return true;
	}
	else if (modifierFlags == Command::MODIFIER_CTRL) {
		*action = new CallbackAction(_T("最大化表示"), [&](Parameter*, String*) -> bool {
			return in->mWorksheet->Activate(true);
		});
		return true;
	}
	return false;
}

HICON WorksheetCommand::GetIcon()
{
	// 拡張子に関連付けられたアイコンを取得
	LPCTSTR fileExt = PathFindExtension(in->mWorksheet->GetWorkbookName().c_str());
	if (_tcslen(fileExt) == 0) {
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return IconLoader::Get()->LoadExtensionIcon(fileExt);
}

launcherapp::core::Command*
WorksheetCommand::Clone()
{
	return new WorksheetCommand(in->mWorksheet);
}

CString WorksheetCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WORKSHEET);
	return TEXT_TYPE;
}

} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

