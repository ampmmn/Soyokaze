#include "pch.h"
#include "PresentationProvider.h"
#include "commands/presentation/Presentations.h"
#include "commands/presentation/PptJumpCommand.h"
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
namespace presentation {

using CommandRepository = launcherapp::core::CommandRepository;

struct PresentationProvider::PImpl
{
	Presentations mPresentations;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PresentationProvider)


PresentationProvider::PresentationProvider() : in(std::make_unique<PImpl>())
{
}

PresentationProvider::~PresentationProvider()
{
}


CString PresentationProvider::GetName()
{
	return _T("Presentation");
}

// 一時的なコマンドの準備を行うための初期化
void PresentationProvider::PrepareAdhocCommands()
{
	in->mPresentations.Load();
}

// 一時的なコマンドを必要に応じて提供する
void PresentationProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	std::vector<SLIDE_ITEM> items;
	in->mPresentations.Query(pattern, items, 16);

	if (items.empty()) {
		return;
	}

	CString filePath = in->mPresentations.GetFilePath();

	for (auto& item : items) {
		int level = item.mMatchLevel;
		commands.Add(CommandQueryItem(level, new PptJumpCommand(filePath, item.mPage, item.mTitle)));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t PresentationProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(PptJumpCommand::TypeDisplayName());
	return 1;
}

} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

