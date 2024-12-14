#include "pch.h"
#include "ActivateWindowProvider.h"
#include "commands/activate_window/WindowList.h"
#include "commands/activate_window/WindowActivateAdhocCommand.h"
#include "commands/activate_window/WindowActivateCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"

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
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnableWindowSwitch = pref->IsEnableWindowSwitch();

	}
	void OnAppExit() override {}


	bool mIsEnableWindowSwitch = false;
	bool mIsFirstCall = true;
	WindowList mWndList;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ActivateWindowProvider)

IMPLEMENT_LOADFROM(ActivateWindowProvider, WindowActivateCommand)

ActivateWindowProvider::ActivateWindowProvider() : in(std::make_unique<PImpl>())
{
}

ActivateWindowProvider::~ActivateWindowProvider()
{
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
bool ActivateWindowProvider::NewDialog(CommandParameter* param)
{
	WindowActivateCommand* newCmd = nullptr;
	if (WindowActivateCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	constexpr bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd, isReloadHotKey);
	return true;
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
		in->mIsEnableWindowSwitch = pref->IsEnableWindowSwitch();
		in->mIsFirstCall = false;
	}

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
		commands.Add(CommandQueryItem(level, new WindowActivateAdhocCommand(hwnd)));
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ActivateWindowProvider::GetOrder() const
{
	return 500;
}

} // end of namespace activate_window
} // end of namespace commands
} // end of namespace launcherapp

