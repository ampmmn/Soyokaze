#include "pch.h"
#include "ControllPanel.h"
#include "commands/controlpanel/ControlPanelCommand.h"
#include "commands/controlpanel/SystemToolCommand.h"
#include "utility/RegistryKey.h"
#include "utility/Path.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/ExpandFunctions.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace controlpanel {

namespace {

struct SystemToolDefinition
{
	LPCTSTR mName;
	LPCTSTR mDescription;
	std::vector<CString> mCommandLine;
};

const SystemToolDefinition SYSTEM_TOOL_DEFINITIONS[] = {
	{_T("環境変数"), _T("環境変数画面を表示する"), {_T("${env SystemRoot}\\system32\\rundll32.exe"), _T("sysdm.cpl,EditEnvironmentVariables")}},
	{_T("システムのプロパティ"), _T("システムのプロパティ画面を表示する"), {_T("${env SystemRoot}\\system32\\rundll32.exe"), _T("shell32.dll,Control_RunDLL"), _T("sysdm.cpl")}},
	{_T("ネットワーク接続"), _T("ネットワーク接続画面を表示する"), {_T("${env SystemRoot}\\system32\\rundll32.exe"), _T("shell32.dll,Control_RunDLL"), _T("ncpa.cpl")}},
	{_T("証明書ストア(現在のユーザー)"), _T("証明書ストア(現在のユーザー)画面を表示する"), {_T("${env SystemRoot}\\system32\\certmgr.msc")}},
	{_T("証明書ストア(ローカルコンピューター)"), _T("証明書ストア(ローカルコンピューター)画面を表示する"), {_T("${env SystemRoot}\\system32\\certlm.msc")}},
	{_T("ユーザー名およびパスワードの保存"), _T("ユーザー名およびパスワードの保存画面を表示する"), {_T("${env SystemRoot}\\system32\\rundll32.exe"), _T("keymgr.dll,KRShowKeyMgr")}},
};

}

struct ControlPanelProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("ControllPanel"));
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);

		for (auto& command : mPanelItems) {
			command->Release();
		}
		mPanelItems.clear();
		for (auto& command : mSystemToolItems) {
			command->Release();
		}
		mSystemToolItems.clear();
	}

	void EnumItems(std::vector<ControlPanelCommand*>& out);
	void EnumSystemToolItems(std::vector<SystemToolCommand*>& out);


	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableControlPanel();

		if (mIsEnable) {
			EnumItems(mPanelItems);
			EnumSystemToolItems(mSystemToolItems);
		}
		else {
			for (auto& command : mPanelItems) {
				command->Release();
			}
			mPanelItems.clear();
			for (auto& command : mSystemToolItems) {
				command->Release();
			}
			mSystemToolItems.clear();
		}

	}
	void OnAppExit() override {}

	bool mIsEnable{false};

	std::vector<ControlPanelCommand*> mPanelItems;
	std::vector<SystemToolCommand*> mSystemToolItems;
};

void ControlPanelProvider::PImpl::EnumItems(std::vector<ControlPanelCommand*>& out)
{
	std::vector<CString> clsIds;
	RegistryKey HKLM(HKEY_LOCAL_MACHINE);
	HKLM.EnumSubKeyNames(_T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\ControlPanel\\NameSpace"), clsIds);

	RegistryKey HKCR(HKEY_CLASSES_ROOT);

	std::vector<ControlPanelCommand*> tmp;

	for (auto& clsId : clsIds) {
		CString subKey(_T("CLSID\\"));
		subKey += clsId;

		// 表示名の取得
		CString tmpName;
		if (HKCR.GetValue(subKey, _T("LocalizedString"), tmpName) == false) {
			continue;
		}

		Path resolvedName;
		SHLoadIndirectString(tmpName, resolvedName, (DWORD)resolvedName.size(), nullptr);

		
		// Tipsテキストの取得
		TCHAR description[1024 + 1] = {};
		if (HKCR.GetValue(subKey, _T("InfoTip"), tmpName)) {
			SHLoadIndirectString(tmpName, description, 1024, nullptr);
		}

		// アイコンの取得
		CString defaultIcon;
		if (HKCR.GetValue(subKey + _T("\\DefaultIcon"), _T(""), defaultIcon) == false) {
			continue;
		}

		// 正規名の取得
		CString appName;
		if (HKCR.GetValue(subKey, _T("System.ApplicationName"), appName) == false) {
			continue;
		}
		tmp.push_back(new ControlPanelCommand((LPCTSTR)resolvedName, defaultIcon, appName, description));
	}

	out.swap(tmp);

	for (auto& command : tmp) {
		command->Release();
	}
}

void ControlPanelProvider::PImpl::EnumSystemToolItems(std::vector<SystemToolCommand*>& out)
{
	std::vector<SystemToolCommand*> tmp;
	for (const auto& definition : SYSTEM_TOOL_DEFINITIONS) {

		// マクロを展開する
		auto commandLine = definition.mCommandLine;
		for (auto& item : commandLine) {
			ExpandMacros(item);
		}

		tmp.push_back(new SystemToolCommand(definition.mName, definition.mDescription, commandLine));
	}
	out.swap(tmp);

	for (auto& command : tmp) {
		command->Release();
	}
}


REGISTER_COMMANDPROVIDER(ControlPanelProvider)

ControlPanelProvider::ControlPanelProvider() : in(std::make_unique<PImpl>())
{
}

ControlPanelProvider::~ControlPanelProvider()
{
}

CString ControlPanelProvider::GetName()
{
	return _T("ControlPanelCommand");
}

// 一時的なコマンドの準備を行うための初期化
void ControlPanelProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	auto pref = AppPreference::Get();
	in->mIsEnable = pref->IsEnableControlPanel();

	// 初回呼び出し時(と有効/無効切り替え時)に一覧生成を行う
	if (in->mIsEnable == false) {
		return;
	}
	in->EnumItems(in->mPanelItems);
	in->EnumSystemToolItems(in->mSystemToolItems);
}

// 一時的なコマンドを必要に応じて提供する
void ControlPanelProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	for (auto& command : in->mPanelItems) {
		int level = command->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}

		command->AddRef();
		commands.Add(CommandQueryItem(level, command));
	}
	for (auto& command : in->mSystemToolItems) {
		int level = command->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}

		command->AddRef();
		commands.Add(CommandQueryItem(level, command));
	}

}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t ControlPanelProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(ControlPanelCommand::TypeDisplayName());
	displayNames.push_back(SystemToolCommand::TypeDisplayName());
	return 2;
}


}
}
}


