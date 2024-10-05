#include "pch.h"
#include "framework.h"
#include "WorksheetCommand.h"
#include "commands/activate_window/ExcelWorksheets.h"
#include "commands/common/CommandParameterFunctions.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace activate_window {

struct WorksheetCommand::PImpl
{
	Worksheet* mWorksheet = nullptr;
};


WorksheetCommand::WorksheetCommand(
	Worksheet* sheet
) : in(std::make_unique<PImpl>())
{
	in->mWorksheet = sheet;
	sheet->AddRef();

	this->mName = sheet->GetWorkbookName() + _T(" - ") + sheet->GetSheetName();
	this->mDescription = sheet->GetSheetName();

}

WorksheetCommand::~WorksheetCommand()
{
	if (in->mWorksheet) {
		in->mWorksheet->Release();
	}
}

CString WorksheetCommand::GetGuideString()
{
	return _T("Enter:シートをアクティブにする");
}

CString WorksheetCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WORKSHEET);
	return TEXT_TYPE;
}

BOOL WorksheetCommand::Execute(Parameter* param)
{
	// Ctrlキーが押されていたら最大化表示する
	bool isShowMaximize = GetModifierKeyState(param, MASK_CTRL) != 0;

	if (in->mWorksheet) {
		return in->mWorksheet->Activate(isShowMaximize);
	}
	return FALSE;
}

HICON WorksheetCommand::GetIcon()
{
	// 拡張子に関連付けられたアイコンを取得
	LPCTSTR fileExt = PathFindExtension(in->mWorksheet->GetWorkbookName());
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

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

