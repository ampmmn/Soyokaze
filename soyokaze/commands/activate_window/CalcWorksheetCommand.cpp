#include "pch.h"
#include "framework.h"
#include "CalcWorksheetCommand.h"
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


struct CalcWorksheetCommand::PImpl
{
	CalcWorksheet* mCalcWorksheet = nullptr;
};

CalcWorksheetCommand::CalcWorksheetCommand(
	CalcWorksheet* sheet
) : in(std::make_unique<PImpl>())
{
	in->mCalcWorksheet = sheet;
	sheet->AddRef();

	this->mName = CString(PathFindFileName(sheet->GetWorkbookName())) + _T(" - ") + sheet->GetSheetName();
	this->mDescription = sheet->GetSheetName();
}


CalcWorksheetCommand::~CalcWorksheetCommand()
{
	if (in->mCalcWorksheet) {
		in->mCalcWorksheet->Release();
	}
}

CString CalcWorksheetCommand::GetTypeDisplayName()
{
	return _T("Calcワークシート");
}

BOOL CalcWorksheetCommand::Execute(const Parameter& param)
{
	// Ctrlキーが押されていたら最大化表示する
	bool isShowMaximize = param.GetNamedParamBool(_T("CtrlKeyPressed"));

	if (in->mCalcWorksheet) {
		return in->mCalcWorksheet->Activate(isShowMaximize);
	}
	return FALSE;
}

HICON CalcWorksheetCommand::GetIcon()
{
	// 拡張子に関連付けられたアイコンを取得
	LPCTSTR fileExt = PathFindExtension(in->mCalcWorksheet->GetWorkbookName());
	if (_tcslen(fileExt) == 0) {
		return IconLoader::Get()->LoadUnknownIcon();
	}
	return IconLoader::Get()->LoadExtensionIcon(fileExt);
}

soyokaze::core::Command*
CalcWorksheetCommand::Clone()
{
	return new CalcWorksheetCommand(in->mCalcWorksheet);
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

