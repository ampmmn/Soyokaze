#include "pch.h"
#include "MMCCommandProvider.h"
#include "commands/mmc/MMCCommand.h"
#include "commands/mmc/MMCSnapins.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace mmc {

using CommandRepository = launcherapp::core::CommandRepository;

struct MMCCommandProvider::PImpl : public AppPreferenceListenerIF
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
		mIsEnable = pref->IsEnableMMCSnapin();
	}

	bool mIsEnable{true};
	std::vector<MMCSnapin> mItems;
	MMCSnapins mSnapins;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(MMCCommandProvider)


MMCCommandProvider::MMCCommandProvider() : in(std::make_unique<PImpl>())
{
}

MMCCommandProvider::~MMCCommandProvider()
{
}

CString MMCCommandProvider::GetName()
{
	return _T("MMCSnapins");
}

// 一時的なコマンドの準備を行うための初期化
void MMCCommandProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void MMCCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsEnable == false) {
		return;
	}

	in->mSnapins.GetSnapins(in->mItems);
	for (auto& item : in->mItems) {

		int level = pattern->Match(item.mDisplayName);
		if (level == Pattern::Mismatch) {
			continue;
		}
		auto cmd = make_refptr<MMCCommand>(item);
		commands.Add(CommandQueryItem(level, cmd.release()));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t MMCCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(MMCCommand::TypeDisplayName());
	return 1;
}


} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

