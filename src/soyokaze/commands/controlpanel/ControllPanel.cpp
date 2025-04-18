#include "pch.h"
#include "ControllPanel.h"
#include "commands/controlpanel/ControlPanelCommand.h"
#include "utility/RegistryKey.h"
#include "utility/Path.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace controlpanel {

struct ControlPanelProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);

		for (auto& command : mPanelItems) {
			command->Release();
		}
		mPanelItems.clear();
	}

	void ControlPanelProvider::PImpl::EnumItems(std::vector<ControlPanelCommand*>& out);


	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableControlPanel();

		if (mIsEnable) {
			EnumItems(mPanelItems);
		}
		else {
			for (auto& command : mPanelItems) {
				command->Release();
			}
			mPanelItems.clear();
		}

	}
	void OnAppExit() override {}

	bool mIsEnable{false};
	bool mIsFirstCall{true};

	std::vector<ControlPanelCommand*> mPanelItems;
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

// 一時的なコマンドを必要に応じて提供する
void ControlPanelProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsEnable = pref->IsEnableControlPanel();
		in->mIsFirstCall = false;

		// 初回呼び出し時(と有効/無効切り替え時)に一覧生成を行う
		if (in->mIsEnable) {
			in->EnumItems(in->mPanelItems);
		}
	}

	for (auto& command : in->mPanelItems) {
		int level = command->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}

		command->AddRef();
		commands.Add(CommandQueryItem(level, command));
	}

}


}
}
}


