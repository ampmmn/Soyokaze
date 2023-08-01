#include "pch.h"
#include "ControllPanel.h"
#include "commands/controlpanel/ControlPanelCommand.h"
#include "RegistryKey.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
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

	bool mIsEnable = false;
	bool mIsFirstCall = true;
	uint32_t mRefCount = 1;

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
		TCHAR resolvedName[MAX_PATH_NTFS];
		SHLoadIndirectString(tmpName, resolvedName, MAX_PATH_NTFS, nullptr);

		
		// Tipsテキストの取得
		TCHAR description[1024];
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

		auto command = new ControlPanelCommand(resolvedName, defaultIcon, appName, description);
		tmp.push_back(command);
	}

	out.swap(tmp);

	for (auto& command : tmp) {
		command->Release();
	}
}


REGISTER_COMMANDPROVIDER(ControlPanelProvider)

ControlPanelProvider::ControlPanelProvider() : in(new PImpl)
{
}

ControlPanelProvider::~ControlPanelProvider()
{
}

// 初回起動の初期化を行う
void ControlPanelProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void ControlPanelProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// 何もしない
}

CString ControlPanelProvider::GetName()
{
	return _T("ControlPanelCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString ControlPanelProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString ControlPanelProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool ControlPanelProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool ControlPanelProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void ControlPanelProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsEnable = pref->IsEnableControlPanel();
		in->mIsFirstCall = false;

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
		commands.push_back(CommandQueryItem(level, command));
	}

}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ControlPanelProvider::GetOrder() const
{
	return 2000;
}

uint32_t ControlPanelProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t ControlPanelProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


}
}
}


