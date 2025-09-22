#include "pch.h"
#include "MSSettingsCommandProvider.h"
#include "commands/mssettings/MSSettingsCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"
#include <fstream>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace mssettings {

using json = nlohmann::json;

struct MSSettingsCommandProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);

		for (auto& command : mItems) {
			command->Release();
		}
		mItems.clear();
	}

	void MSSettingsCommandProvider::PImpl::EnumItems(std::vector<MSSettingsCommand*>& out);


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
		mIsEnable = pref->IsEnableMSSettings();

		if (mIsEnable) {
			EnumItems(mItems);
		}
		else {
			for (auto& command : mItems) {
				command->Release();
			}
			mItems.clear();
		}
	}

	bool mIsEnable{false};

	std::vector<MSSettingsCommand*> mItems;
};

void MSSettingsCommandProvider::PImpl::EnumItems(std::vector<MSSettingsCommand*>& out)
{
	Path settingsFilePath(Path::MODULEFILEDIR);
	settingsFilePath.Append(_T("files\\ms-settings\\default.json"));

	if (Path::FileExists(settingsFilePath) == false) {
		spdlog::error(_T("ms-settings settings list does not exist. {}"), (LPCTSTR)settingsFilePath);
		return;
	}

	try {
		std::vector<MSSettingsCommand*> tmp;

		std::ifstream f((LPCTSTR)settingsFilePath);
		json j = json::parse(f);
		for (auto it = j.begin(); it != j.end(); ++it) {

			std::string s(it.key());
			CString scheme;
			UTF2UTF(s, scheme);

			auto value = it.value();
			if (value.contains("Enable") && value["Enable"] == false) {
				continue;
			}
			if (value.contains("Page") == false || value.contains("Category") == false) {
				continue;
			}

			s = value["Category"];	
			CString category;
			UTF2UTF(s, category);
			s = value["Page"];	
			CString pageName;
			UTF2UTF(s, pageName);

			tmp.push_back(new MSSettingsCommand(scheme, category, pageName));
		}

		// 名前で昇順ソート
		std::sort(tmp.begin(), tmp.end(), [](MSSettingsCommand* l, MSSettingsCommand* r) {
				return l->GetName() < r->GetName();
		});

		// 結果を返す
		out.swap(tmp);

		for (auto cmd : tmp) {
			cmd->Release();
		}
	}
	catch(...) {
		spdlog::error(_T("Failed to parse json. {}"), (LPCTSTR)settingsFilePath);
		return;
	}
}


REGISTER_COMMANDPROVIDER(MSSettingsCommandProvider)

MSSettingsCommandProvider::MSSettingsCommandProvider() : in(std::make_unique<PImpl>())
{
}

MSSettingsCommandProvider::~MSSettingsCommandProvider()
{
}

CString MSSettingsCommandProvider::GetName()
{
	return _T("MSSettingsCommand");
}

// 一時的なコマンドの準備を行うための初期化
void MSSettingsCommandProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void MSSettingsCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	for (auto& command : in->mItems) {
		int level = command->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}

		command->AddRef();
		commands.Add(CommandQueryItem(level, command));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t MSSettingsCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(MSSettingsCommand::TypeDisplayName());
	return 1;
}


}
}
}


