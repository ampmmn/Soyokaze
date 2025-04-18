#include "pch.h"
#include "ActivateWorksheetProvider.h"
#include "commands/activate_worksheet/WorksheetCommand.h"
#include "commands/activate_worksheet/ExcelWorksheets.h"
#include "commands/activate_worksheet/CalcWorksheets.h"
#include "commands/activate_worksheet/CalcWorksheetCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_worksheet {

struct ActivateWorksheetProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnableWorksheet = pref->IsEnableExcelWorksheet();

	}
	void OnAppExit() override {}


	//
	bool mIsEnableWorksheet {false};
	bool mIsFirstCall {true};

	WorkSheets mWorksheets;
	CalcWorkSheets mCalcWorksheets;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ActivateWorksheetProvider)

ActivateWorksheetProvider::ActivateWorksheetProvider() : in(std::make_unique<PImpl>())
{
}

ActivateWorksheetProvider::~ActivateWorksheetProvider()
{
}

CString ActivateWorksheetProvider::GetName()
{
	return _T("ActiveWorksheetCommand");
}

// 一時的なコマンドを必要に応じて提供する
void ActivateWorksheetProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsEnableWorksheet = pref->IsEnableExcelWorksheet();
		in->mIsFirstCall = false;
	}

	QueryAdhocCommandsForWorksheets(pattern, commands);
}

void ActivateWorksheetProvider::QueryAdhocCommandsForWorksheets(
	Pattern* pattern,
	launcherapp::CommandQueryItemList& commands
)
{
	if (in->mIsEnableWorksheet == false) {
		return ;
	}

	std::vector<Worksheet*> sheets;
	in->mWorksheets.GetWorksheets(sheets);

	for (auto& sheet : sheets) {
		CString str = sheet->GetWorkbookName() + _T(" - ") + sheet->GetSheetName();
		int level = pattern->Match(str);
		if (level != Pattern::Mismatch) {

			commands.Add(CommandQueryItem(level, new WorksheetCommand(sheet)));
		}
		sheet->Release();
	}

	std::vector<CalcWorksheet*> calcSheets;
	in->mCalcWorksheets.GetWorksheets(calcSheets);

	for (auto& sheet : calcSheets) {
		CString str = sheet->GetWorkbookName() + _T(" - ") + sheet->GetSheetName();
		int level = pattern->Match(str);
		if (level != Pattern::Mismatch) {

			commands.Add(CommandQueryItem(level, new CalcWorksheetCommand(sheet)));
		}
		sheet->Release();
	}
}

} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

