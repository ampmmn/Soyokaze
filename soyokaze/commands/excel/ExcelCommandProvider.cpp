#include "pch.h"
#include "ExcelCommandProvider.h"
#include "commands/excel/WorksheetCommand.h"
#include "commands/excel/ExcelWorksheets.h"
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
namespace excel {


using CommandRepository = soyokaze::core::CommandRepository;

struct ExcelCommandProvider::PImpl : public AppPreferenceListenerIF
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
	}
	void OnAppExit() override {}


	//
	bool mIsEnableWorksheet;

	bool mIsFirstCall;

	WorkSheets mWorksheets;

	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ExcelCommandProvider)


ExcelCommandProvider::ExcelCommandProvider() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mIsEnableWorksheet = false;
	in->mIsFirstCall = true;
}

ExcelCommandProvider::~ExcelCommandProvider()
{
}

// 初回起動の初期化を行う
void ExcelCommandProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void ExcelCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// 何もしない
}

CString ExcelCommandProvider::GetName()
{
	return _T("ExcelCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString ExcelCommandProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString ExcelCommandProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool ExcelCommandProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool ExcelCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void ExcelCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsEnableWorksheet = pref->IsEnableExcelWorksheet();
		in->mIsFirstCall = false;
	}

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

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ExcelCommandProvider::ExcelCommandProvider::GetOrder() const
{
	return 2000;
}

uint32_t ExcelCommandProvider::ExcelCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t ExcelCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace excel
} // end of namespace commands
} // end of namespace soyokaze

