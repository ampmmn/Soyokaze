#include "pch.h"
#include "VSCodeProvider.h"
#include "commands/vscode/VSCodeCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace vscode {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct VSCodeProvider::PImpl :
	public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {}

	void Load() {
		mInternalCommand.Load();
	}

	VSCodeCommand mInternalCommand;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(VSCodeProvider)


VSCodeProvider::VSCodeProvider() : in(new PImpl())
{
}

VSCodeProvider::~VSCodeProvider()
{
}

CString VSCodeProvider::GetName()
{
	return _T("VSCodeCommand");
}

// 一時的なコマンドの準備を行うための初期化
void VSCodeProvider::PrepareAdhocCommands()
{
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void VSCodeProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	in->mInternalCommand.QueryCandidates(pattern, commands);
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t VSCodeProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	return VSCodeCommand::EnumCommandDisplayNames(displayNames);
}


}}} // end of namespace launcherapp::commands::vscode

