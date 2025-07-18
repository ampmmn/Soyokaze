#include "pch.h"
#include "UnitConvertProvider.h"
#include "commands/unitconvert/InchAdhocCommand.h"
#include "commands/unitconvert/EraNameWJCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace unitconvert {

using CommandRepository = launcherapp::core::CommandRepository;
using CommandPtr = RefPtr<launcherapp::core::Command>;

struct UnitConvertProvider::PImpl : public AppPreferenceListenerIF
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
		Reload();
	}
	void OnAppExit() override {}

	void Reload()
	{
		// FIXME:必要に応じて実装
	}

	std::list<CommandPtr> mConverterCommands;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(UnitConvertProvider)


UnitConvertProvider::UnitConvertProvider() : in(std::make_unique<PImpl>())
{
	in->mConverterCommands.push_back(CommandPtr(new InchAdhocCommand()));
	in->mConverterCommands.push_back(CommandPtr(new EraNameWJCommand()));
}

UnitConvertProvider::~UnitConvertProvider()
{
}

CString UnitConvertProvider::GetName()
{
	return _T("UnitConvert");
}

// 一時的なコマンドの準備を行うための初期化
void UnitConvertProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	in->Reload();
}

// 一時的なコマンドを必要に応じて提供する
void UnitConvertProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	for (auto& cmdPtr : in->mConverterCommands) {
		int level = cmdPtr->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}
		cmdPtr->AddRef();
		commands.Add(CommandQueryItem(level, cmdPtr.get()));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t UnitConvertProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(InchAdhocCommand::TypeDisplayName());
	displayNames.push_back(EraNameWJCommand::TypeDisplayName());
	return 2;
}

} // end of namespace unitconvert
} // end of namespace commands
} // end of namespace launcherapp

