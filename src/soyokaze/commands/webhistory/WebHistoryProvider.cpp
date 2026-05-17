#include "pch.h"
#include "WebHistoryProvider.h"
#include "commands/webhistory/WebHistoryCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <list>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace webhistory {

using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct WebHistoryProvider::PImpl :
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
		if (mInternalCommand.get() == nullptr) {
			mInternalCommand.reset(new WebHistoryCommand());
		}
		mInternalCommand->Load();
	}

	std::unique_ptr<WebHistoryCommand> mInternalCommand;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(WebHistoryProvider)


WebHistoryProvider::WebHistoryProvider() : in(new PImpl())
{
}

WebHistoryProvider::~WebHistoryProvider()
{
}

CString WebHistoryProvider::GetName()
{
	return _T("WebHistoryCommand");
}

// 一時的なコマンドの準備を行うための初期化
void WebHistoryProvider::PrepareAdhocCommands()
{
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void WebHistoryProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	if (in->mInternalCommand.get() == nullptr) {
		in->mInternalCommand.reset(new WebHistoryCommand());
	}
	in->mInternalCommand->QueryCandidates(pattern, commands);
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t WebHistoryProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	return WebHistoryCommand::EnumCommandDisplayNames(displayNames);
}


}}} // end of namespace launcherapp::commands::webhistory

