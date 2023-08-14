#include "pch.h"
#include "framework.h"
#include "WorksheetCommand.h"
#include "commands/activate_window/ExcelWorksheets.h"
#include "IconLoader.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace activate_window {


struct WorksheetCommand::PImpl
{
	Worksheet* mWorksheet;
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

CString WorksheetCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WORKSHEET);
	return TEXT_TYPE;
}

BOOL WorksheetCommand::Execute(const Parameter& param)
{
	// Ctrlキーが押されていたら最大化表示する
	bool isShowMaximize = param.GetNamedParamBool(_T("CtrlKeyPressed"));

	return in->mWorksheet->Activate(isShowMaximize);
}

HICON WorksheetCommand::GetIcon()
{
	const auto& path = in->mWorksheet->GetAppPath();
	return IconLoader::Get()->LoadIconFromPath(path);
}

soyokaze::core::Command*
WorksheetCommand::Clone()
{
	return new WorksheetCommand(in->mWorksheet);
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

