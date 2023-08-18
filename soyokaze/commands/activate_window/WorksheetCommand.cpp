#include "pch.h"
#include "framework.h"
#include "WorksheetCommand.h"
#include "commands/activate_window/ExcelWorksheets.h"
#include "commands/activate_window/CalcWorksheets.h"
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
	Worksheet* mWorksheet = nullptr;
	CalcWorksheet* mCalcWorksheet = nullptr;
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

WorksheetCommand::WorksheetCommand(
	CalcWorksheet* sheet
) : in(std::make_unique<PImpl>())
{
	in->mCalcWorksheet = sheet;
	sheet->AddRef();

	this->mName = sheet->GetWorkbookName() + _T(" - ") + sheet->GetSheetName();
	this->mDescription = sheet->GetSheetName();

}


WorksheetCommand::~WorksheetCommand()
{
	if (in->mWorksheet) {
		in->mWorksheet->Release();
	}
	if (in->mCalcWorksheet) {
		in->mCalcWorksheet->Release();
	}
}

CString WorksheetCommand::GetTypeDisplayName()
{
	if (in->mWorksheet) {
		static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WORKSHEET);
		return TEXT_TYPE;
	}
	else {
		return _T("Calcワークシート");
	}
}

BOOL WorksheetCommand::Execute(const Parameter& param)
{
	// Ctrlキーが押されていたら最大化表示する
	bool isShowMaximize = param.GetNamedParamBool(_T("CtrlKeyPressed"));

	if (in->mWorksheet) {
		return in->mWorksheet->Activate(isShowMaximize);
	}
	else if (in->mCalcWorksheet) {
		return in->mCalcWorksheet->Activate(isShowMaximize);
	}
	return FALSE;
}

HICON WorksheetCommand::GetIcon()
{
	if (in->mWorksheet) {
		const auto& path = in->mWorksheet->GetAppPath();
		return IconLoader::Get()->LoadIconFromPath(path);
	}
	else {
		return IconLoader::Get()->LoadDefaultIcon();
	}
}

soyokaze::core::Command*
WorksheetCommand::Clone()
{
	if (in->mWorksheet) {
		return new WorksheetCommand(in->mWorksheet);
	}
	else {
		return new WorksheetCommand(in->mCalcWorksheet);
	}
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

