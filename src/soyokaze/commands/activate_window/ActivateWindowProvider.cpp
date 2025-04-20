#include "pch.h"
#include "ActivateWindowProvider.h"
#include "commands/activate_window/WindowList.h"
#include "commands/activate_window/WindowActivateAdhocCommand.h"
#include "commands/activate_window/WindowActivateCommand.h"
#include "commands/activate_window/WindowActivateAdhocNameDialog.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "resource.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace activate_window {

using CommandRepository = launcherapp::core::CommandRepository;

struct ActivateWindowProvider::PImpl :
 	public AppPreferenceListenerIF,
 	public MenuEventListener
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void Reload() {
		auto pref = AppPreference::Get();
		mIsEnableWindowSwitch = pref->IsEnableWindowSwitch();
		mPrefix = pref->GetWindowSwitchPrefix();
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Reload();
	}
	void OnAppExit() override {}

// MenuEventListener
	void OnRequestPutName(HWND hwnd)
	{
		AdhocNameDialog dlg;

		// 既に設定されている名前がある場合はそれをダイアログにセットする
		auto it = mAdhocNameMap.find(hwnd);
		if (it != mAdhocNameMap.end()) {
			dlg.SetName(it->second);
		}

		if (dlg.DoModal() != IDOK) {
			return ;
		}
		auto& name = dlg.GetName();
		if (name.IsEmpty() && it != mAdhocNameMap.end()) {
			// 変更後の名前が空で、以前に設定されていた名前がある場合は関連付けを削除する
			mAdhocNameMap.erase(it);
			return;
		}
		if (name.IsEmpty() == FALSE) {
			// 新たに名前を登録する
			mAdhocNameMap[hwnd] = name;
		}
	}
	void OnRequestClose(HWND hwnd)
	{
		auto it = mAdhocNameMap.find(hwnd);
		if (it == mAdhocNameMap.end()) {
			return;
		}
		mAdhocNameMap.erase(it);
	}

	bool mIsEnableWindowSwitch{false};
	bool mIsFirstCall{true};
	WindowList mWndList;
	CString mPrefix;

	std::map<HWND, CString> mAdhocNameMap;
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
	WindowActivateCommand* newCmd{nullptr};
	if (WindowActivateCommand::NewDialog(param, &newCmd) == false) {
		return false;
	}

	constexpr bool isReloadHotKey{true};
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
		in->Reload();
		in->mIsFirstCall = false;
	}

	// 機能を利用しない場合は抜ける
	if (in->mIsEnableWindowSwitch == false) {
		return ;
	}
	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	bool hasPrefix =  prefix.IsEmpty() == FALSE;
	int offset = hasPrefix ? 1 : 0;

	auto it = in->mAdhocNameMap.begin();
	while(it != in->mAdhocNameMap.end()) { 
		auto& name = it->second;
		int level = pattern->Match(name, offset);
		if (level == Pattern::Mismatch) {
			it++;
			continue;
		}

		auto hwnd = it->first;
		if (IsWindow(hwnd) == FALSE) {
			it = in->mAdhocNameMap.erase(it);
			continue;
		}

		// プレフィックスがある場合は最低でも前方一致とする
		if (hasPrefix && level == Pattern::PartialMatch) {
			level = Pattern::FrontMatch;
		}

		auto cmd = new WindowActivateAdhocCommand(hwnd);
		cmd->SetListener(in.get());
		commands.Add(CommandQueryItem(level, cmd));
		it++;
	}

	std::vector<HWND> windowHandles;
	in->mWndList.EnumWindowHandles(windowHandles);
	SPDLOG_DEBUG(_T("Window count : {}"), windowHandles.size());


	// プレフィックスあり、かつ、後続キーワードなしの場合は全て列挙
	bool shouldEnumAll = pattern->GetWordCount() == 1;

	TCHAR caption[256];
	for (auto hwnd : windowHandles) {
		GetWindowText(hwnd, caption, 256);

		// ウインドウテキストを持たないものを除外する
		if (caption[0] == _T('\0')) {
			continue;
		}

		int level = Pattern::FrontMatch;

		if (shouldEnumAll == false) {
			level = pattern->Match(caption, offset);
			if (level == Pattern::Mismatch) {
				continue;
			}
		}
		auto cmd = new WindowActivateAdhocCommand(hwnd);
		cmd->SetListener(in.get());
		commands.Add(CommandQueryItem(level, cmd));
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

