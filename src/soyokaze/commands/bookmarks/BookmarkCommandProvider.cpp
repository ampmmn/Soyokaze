#include "pch.h"
#include "BookmarkCommandProvider.h"
#include "commands/bookmarks/BookmarkCommand.h"
#include "commands/bookmarks/URLCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace bookmarks {

using CommandRepository = launcherapp::core::CommandRepository;

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct BookmarkCommandProvider::PImpl :
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

	BookmarkCommand mInternalCommand;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(BookmarkCommandProvider)


BookmarkCommandProvider::BookmarkCommandProvider() : in(new PImpl)
{
}

BookmarkCommandProvider::~BookmarkCommandProvider()
{
}

CString BookmarkCommandProvider::GetName()
{
	return _T("BookmarkCommand");
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t BookmarkCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(URLCommand::TypeDisplayName(_T("Chrome")));
	displayNames.push_back(URLCommand::TypeDisplayName(_T("Edge")));
	return 2;
}

// 一時的なコマンドの準備を行うための初期化
void BookmarkCommandProvider::PrepareAdhocCommands()
{
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void BookmarkCommandProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	in->mInternalCommand.QueryCandidates(pattern, commands);
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

