#include "pch.h"
#include "framework.h"
#include "CalcWorksheetCommand.h"
#include "commands/activate_worksheet/CalcWorksheets.h"
#include "commands/common/CommandParameterFunctions.h"
#include "actions/builtin/CallbackAction.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

using namespace launcherapp::commands::common;
using namespace launcherapp::actions::builtin;

struct CalcWorksheetCommand::PImpl
{
	CalcWorksheet* mCalcWorksheet{nullptr};
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(CalcWorksheetCommand)

CalcWorksheetCommand::CalcWorksheetCommand(
	CalcWorksheet* sheet
) : in(std::make_unique<PImpl>())
{
	in->mCalcWorksheet = sheet;
	sheet->AddRef();

	this->mName = fmt::format(_T("{} ({})"), sheet->GetSheetName(), PathFindFileName(sheet->GetWorkbookName().c_str())).c_str();
	this->mDescription = sheet->GetSheetName().c_str();
}


CalcWorksheetCommand::~CalcWorksheetCommand()
{
	if (in->mCalcWorksheet) {
		in->mCalcWorksheet->Release();
	}
}

CString CalcWorksheetCommand::GetGuideString()
{
	return _T("⏎:表示 C-⏎:最大化表示");
}

CString CalcWorksheetCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool CalcWorksheetCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags & Command::MODIFIER_CTRL) {
		*action = new CallbackAction(_T("最大化表示"), [&](Parameter*, String* errMsg) -> bool {
			bool result = in->mCalcWorksheet->Activate(true);
			if (result == false && errMsg) {
				*errMsg = in->mCalcWorksheet->GetErrorMessage();
			}
			return result;
		});
	}
	else {
		*action = new CallbackAction(_T("表示"), [&](Parameter*, String* errMsg) -> bool {
			bool result = in->mCalcWorksheet->Activate(false);
			if (result == false && errMsg) {
				*errMsg = in->mCalcWorksheet->GetErrorMessage();
			}
			return result;
		});
	}
	return true;
}


HICON CalcWorksheetCommand::GetIcon()
{
	// 拡張子に関連付けられたアイコンを取得
	LPCTSTR fileExt = PathFindExtension(in->mCalcWorksheet->GetWorkbookName().c_str());
	if (_tcslen(fileExt) == 0) {
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return IconLoader::Get()->LoadExtensionIcon(fileExt);
}

launcherapp::core::Command*
CalcWorksheetCommand::Clone()
{
	return new CalcWorksheetCommand(in->mCalcWorksheet);
}

CString CalcWorksheetCommand::TypeDisplayName()
{
	return _T("Calcワークシート");
}

} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

