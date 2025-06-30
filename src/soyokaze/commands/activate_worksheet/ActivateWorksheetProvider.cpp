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
		mPrefix = pref->GetWorksheetSwitchPrefix();

	}
	void OnAppExit() override {}


	//
	bool mIsEnableWorksheet {false};
	CString mPrefix;

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

// 一時的なコマンドの準備を行うための初期化
void ActivateWorksheetProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	auto pref = AppPreference::Get();
	in->mIsEnableWorksheet = pref->IsEnableExcelWorksheet();
	in->mPrefix = pref->GetWorksheetSwitchPrefix();
}

// 一時的なコマンドを必要に応じて提供する
void ActivateWorksheetProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	// 機能を利用しない場合は抜ける
	if (in->mIsEnableWorksheet == false) {
		return ;
	}
	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	bool hasPrefix =  prefix.IsEmpty() == FALSE;
	int offset = hasPrefix ? 1 : 0;

	std::vector<Worksheet*> sheets;
	in->mWorksheets.GetWorksheets(sheets);

	for (auto& sheet : sheets) {
		int levelw = pattern->Match(sheet->GetWorkbookName().c_str(), offset);
		int levels = pattern->Match(sheet->GetSheetName().c_str(), offset);
		if (levelw != Pattern::Mismatch || levels != Pattern::Mismatch) {

			int level = (std::max)(levelw, levels);
			// プレフィックスがある場合は最低でも前方一致とする
			if (hasPrefix && level == Pattern::PartialMatch) {
				level = Pattern::FrontMatch;
			}

			commands.Add(CommandQueryItem(level, new WorksheetCommand(sheet)));
		}
		sheet->Release();
	}

	std::vector<CalcWorksheet*> calcSheets;
	in->mCalcWorksheets.GetWorksheets(calcSheets);

	for (auto& sheet : calcSheets) {
		int levelw = pattern->Match(sheet->GetWorkbookName().c_str(), offset);
		int levels = pattern->Match(sheet->GetSheetName().c_str(), offset);
		if (levelw != Pattern::Mismatch || levels != Pattern::Mismatch) {

			int level = (std::max)(levelw, levels);
			// プレフィックスがある場合は最低でも前方一致とする
			if (hasPrefix && level == Pattern::PartialMatch) {
				level = Pattern::FrontMatch;
			}

			commands.Add(CommandQueryItem(level, new CalcWorksheetCommand(sheet)));
		}
		sheet->Release();
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t ActivateWorksheetProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(WorksheetCommand::TypeDisplayName());
	displayNames.push_back(CalcWorksheetCommand::TypeDisplayName());
	return 2;
}

} // end of namespace activate_worksheet
} // end of namespace commands
} // end of namespace launcherapp

