#include "pch.h"
#include "UWPCommandProvider.h"
#include "commands/uwp/UWPApplicationItem.h"
#include "commands/uwp/UWPCommand.h"
#include "commands/uwp/UWPApplications.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace uwp {

using CommandRepository = launcherapp::core::CommandRepository;

struct UWPCommandProvider::PImpl : public AppPreferenceListenerIF
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
		Load();
	}
	void OnAppExit() override {}

	void Load()
	{
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableUWP();
	}

	bool mIsEnable{true};
	std::vector<ItemPtr> mItems;
	UWPApplications mUWPApps;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(UWPCommandProvider)


UWPCommandProvider::UWPCommandProvider() : in(std::make_unique<PImpl>())
{
}

UWPCommandProvider::~UWPCommandProvider()
{
}

CString UWPCommandProvider::GetName()
{
	return _T("UWPApps");
}

// 一時的なコマンドの準備を行うための初期化
void UWPCommandProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void UWPCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsEnable == false) {
		return;
	}

	in->mUWPApps.GetApplications(in->mItems);

	for (auto& item : in->mItems) {

		int level = pattern->Match(item->mName);
		if (level == Pattern::Mismatch) {
			continue;
		}
		auto cmd = make_refptr<UWPCommand>(item);

		commands.Add(CommandQueryItem(level, cmd.release()));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t UWPCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(UWPCommand::TypeDisplayName());
	return 1;
}


} // end of namespace uwp
} // end of namespace commands
} // end of namespace launcherapp

