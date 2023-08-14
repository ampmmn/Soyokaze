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
	CString mName;
	CString mDescription;
	Worksheet* mWorksheet;

	uint32_t mRefCount;
};


WorksheetCommand::WorksheetCommand(
	Worksheet* sheet
) : in(new PImpl)
{
	in->mRefCount = 1;
	in->mWorksheet = sheet;
	sheet->AddRef();

	in->mName = sheet->GetWorkbookName() + _T(" - ") + sheet->GetSheetName();
	in->mDescription = sheet->GetSheetName();

}

WorksheetCommand::~WorksheetCommand()
{
	if (in->mWorksheet) {
		in->mWorksheet->Release();
	}
}

CString WorksheetCommand::GetName()
{
	return in->mName;
}

CString WorksheetCommand::GetDescription()
{
	return in->mDescription;
}

CString WorksheetCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMAND_WORKSHEET);
	return TEXT_TYPE;
}

BOOL WorksheetCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL WorksheetCommand::Execute(const Parameter& param)
{
	// Ctrlキーが押されていたら最大化表示する
	bool isShowMaximize = param.GetNamedParamBool(_T("CtrlKeyPressed"));

	return in->mWorksheet->Activate(isShowMaximize);
}

CString WorksheetCommand::GetErrorString()
{
	return _T("");
}

HICON WorksheetCommand::GetIcon()
{
	const auto& path = in->mWorksheet->GetAppPath();
	return IconLoader::Get()->LoadIconFromPath(path);
}

int WorksheetCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool WorksheetCommand::IsEditable()
{
	return false;
}

int WorksheetCommand::EditDialog(const Parameter* param)
{
	// 実装なし
	return -1;
}

soyokaze::core::Command*
WorksheetCommand::Clone()
{
	return new WorksheetCommand(in->mWorksheet);
}

bool WorksheetCommand::Save(CommandFile* cmdFile)
{
	// 非サポート
	return false;
}

uint32_t WorksheetCommand::AddRef()
{
	return ++(in->mRefCount);
}

uint32_t WorksheetCommand::Release()
{
	auto n = --(in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

