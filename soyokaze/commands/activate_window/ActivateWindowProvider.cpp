#include "pch.h"
#include "ActivateWindowProvider.h"
#include "commands/activate_window/WorksheetCommand.h"
#include "commands/activate_window/WindowActivateCommand.h"
#include "commands/activate_window/ExcelWorksheets.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace activate_window {


using CommandRepository = soyokaze::core::CommandRepository;

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

	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ActivateWindowProvider)


ActivateWindowProvider::ActivateWindowProvider() : in(new PImpl)
{
	in->mRefCount = 1;
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
	// 何もしない
}


// コマンドの読み込み
void ActivateWindowProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// 何もしない
}

CString ActivateWindowProvider::GetName()
{
	return _T("ExcelCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString ActivateWindowProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString ActivateWindowProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool ActivateWindowProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool ActivateWindowProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void ActivateWindowProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
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
	return 2000;
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

void ActivateWindowProvider::QueryAdhocCommandsForWorksheets(Pattern* pattern, std::vector<CommandQueryItem>& commands)
{
	if (in->mIsEnableWorksheet == false) {
		return ;
	}

	std::vector<Worksheet*> sheets;
	in->mWorksheets.GetWorksheets(sheets);

	for (auto sheet : sheets) {
		CString str = sheet->GetWorkbookName() + _T(" - ") + sheet->GetSheetName();
		int level = pattern->Match(str);
		if (level != Pattern::Mismatch) {

			commands.push_back(CommandQueryItem(level, new WorksheetCommand(sheet)));
		}
		sheet->Release();
	}
}

// ウインドウ切り替え用コマンド生成
void ActivateWindowProvider::QueryAdhocCommandsForWindows(Pattern* pattern, std::vector<CommandQueryItem>& commands)
{
	if (in->mIsEnableWindowSwitch == false) {
		return ;
	}

	struct local_param {
		static BOOL CALLBACK OnEnumWindows(HWND hwnd, LPARAM lParam) {

			LONG_PTR style = GetWindowLongPtr(hwnd, GWL_STYLE);
			LONG_PTR styleRequired = (WS_CAPTION | WS_VISIBLE);

			if ((style & styleRequired) != styleRequired) {
				// 非表示のウインドウと、タイトルを持たないウインドウは対象外
				return TRUE;
			}
			auto param = (local_param*)lParam;
			param->mCandidates.push_back(hwnd);

			return TRUE;
		}

		std::vector<HWND> mCandidates;
	} param;

	EnumWindows(local_param::OnEnumWindows, (LPARAM)&param);

	TCHAR caption[256];
	for (auto hwnd : param.mCandidates) {
		GetWindowText(hwnd, caption, 256);

		int level = pattern->Match(caption);
		if (level == Pattern::Mismatch) {
			continue;
		}
		commands.push_back(CommandQueryItem(level, new WindowActivateCommand(hwnd)));
	}
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace soyokaze

