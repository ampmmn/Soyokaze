#include "pch.h"
#include "ActivateWindowProvider.h"
#include "commands/activate_window/WindowList.h"
#include "commands/activate_window/WorksheetCommand.h"
#include "commands/activate_window/WindowActivateAdhocCommand.h"
#include "commands/activate_window/WindowActivateCommand.h"
#include "commands/activate_window/ExcelWorksheets.h"
#include "commands/activate_window/CalcWorksheets.h"
#include "commands/activate_window/CalcWorksheetCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_window {

using CommandRepository = launcherapp::core::CommandRepository;

struct ActivateWindowProvider::PImpl : public AppPreferenceListenerIF
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
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnableWorksheet = pref->IsEnableExcelWorksheet();
		mIsEnableWindowSwitch = pref->IsEnableWindowSwitch();

	}
	void OnAppExit() override {}


	//
	bool mIsEnableWorksheet;

	bool mIsEnableWindowSwitch;

	bool mIsFirstCall;

	WorkSheets mWorksheets;
	CalcWorkSheets mCalcWorksheets;

	WindowList mWndList;

	uint32_t mRefCount = 1;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ActivateWindowProvider)


ActivateWindowProvider::ActivateWindowProvider() : in(std::make_unique<PImpl>())
{
	in->mIsEnableWorksheet = false;
	in->mIsEnableWindowSwitch = false;
	in->mIsFirstCall = true;
}

ActivateWindowProvider::~ActivateWindowProvider()
{
}

// 初回起動の初期化を行う
void ActivateWindowProvider::OnFirstBoot()
{
}

// コマンドの読み込み
void ActivateWindowProvider::LoadCommands(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto cmdRepo = CommandRepository::GetInstance();

	int entries = cmdFile->GetEntryCount();
	for (int i = 0; i < entries; ++i) {

		auto entry = cmdFile->GetEntry(i);
		if (cmdFile->IsUsedEntry(entry)) {
			// 既にロード済(使用済)のエントリ
			continue;
		}

		WindowActivateCommand* command = nullptr;
		if (WindowActivateCommand::LoadFrom(cmdFile, entry, &command) == false) {
			if (command) {
				command->Release();
			}
			continue;
		}

		// 登録
		cmdRepo->RegisterCommand(command);

		// 使用済みとしてマークする
		cmdFile->MarkAsUsed(entry);
	}
}


CString ActivateWindowProvider::GetName()
{
	return _T("ActiveWindowCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString ActivateWindowProvider::GetDisplayName()
{
	return CString((LPCTSTR)IDS_COMMANDNAME_WINDOWACTIVATE);
}

// コマンドの種類の説明を示す文字列を取得
CString ActivateWindowProvider::GetDescription()
{
	CString description((LPCTSTR)IDS_DESCRIPTION_WINDOWACTIVATE);
	description += _T("\n");
	description += _T("ウインドウ切り替え処理に対して、任意のキーワードを設定したり、\n");
	description += _T("ホットキーを設定することができます。\n");
	return description;
}

// コマンド新規作成ダイアログ
bool ActivateWindowProvider::NewDialog(const CommandParameter* param)
{
	WindowActivateCommand* newCmd = nullptr;
	if (WindowActivateCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}
	CommandRepository::GetInstance()->RegisterCommand(newCmd);
	return true;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool ActivateWindowProvider::IsPrivate() const
{
	return false;
}

// 一時的なコマンドを必要に応じて提供する
void ActivateWindowProvider::QueryAdhocCommands(
	Pattern* pattern,
 	launcherapp::CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsEnableWorksheet = pref->IsEnableExcelWorksheet();
		in->mIsEnableWindowSwitch = pref->IsEnableWindowSwitch();
		in->mIsFirstCall = false;
	}

	QueryAdhocCommandsForWorksheets(pattern, commands);
	QueryAdhocCommandsForWindows(pattern, commands);
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ActivateWindowProvider::GetOrder() const
{
	return 500;
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool ActivateWindowProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	return true;
}

uint32_t ActivateWindowProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t ActivateWindowProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

void ActivateWindowProvider::QueryAdhocCommandsForWorksheets(
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

			commands.push_back(CommandQueryItem(level, new WorksheetCommand(sheet)));
		}
		sheet->Release();
	}

	std::vector<CalcWorksheet*> calcSheets;
	in->mCalcWorksheets.GetWorksheets(calcSheets);

	for (auto& sheet : calcSheets) {
		CString str = sheet->GetWorkbookName() + _T(" - ") + sheet->GetSheetName();
		int level = pattern->Match(str);
		if (level != Pattern::Mismatch) {

			commands.push_back(CommandQueryItem(level, new CalcWorksheetCommand(sheet)));
		}
		sheet->Release();
	}
}

// ウインドウ切り替え用コマンド生成
void ActivateWindowProvider::QueryAdhocCommandsForWindows(
	Pattern* pattern,
	launcherapp::CommandQueryItemList& commands
)
{
	if (in->mIsEnableWindowSwitch == false) {
		return ;
	}

	std::vector<HWND> windowHandles;
	in->mWndList.EnumWindowHandles(windowHandles);
	SPDLOG_DEBUG(_T("Window count : {}"), windowHandles.size());

	TCHAR caption[256];
	for (auto hwnd : windowHandles) {
		GetWindowText(hwnd, caption, 256);

		// ウインドウテキストを持たないものを除外する
		if (caption[0] == _T('\0')) {
			continue;
		}

		int level = pattern->Match(caption);
		if (level == Pattern::Mismatch) {
			continue;
		}
		commands.push_back(CommandQueryItem(level, new WindowActivateAdhocCommand(hwnd)));
	}
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

