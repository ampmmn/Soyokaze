#include "pch.h"
#include "SpecialFolderFilesCommandProvider.h"
#include "commands/specialfolderfiles/SpecialFolderFileCommand.h"
#include "commands/specialfolderfiles/SpecialFolderFiles.h"
#include "core/CommandRepository.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace specialfolderfiles {

using CommandRepository = soyokaze::core::CommandRepository;

struct SpecialFolderFilesCommandProvider::PImpl : public AppPreferenceListenerIF
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
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnableSpecialFolder = pref->IsEnableSpecialFolder();
		mIsEnableDesktop = pref->IsEnableDesktop();
	}
	void OnAppExit() override {}

	bool mIsEnableSpecialFolder = true;
	bool mIsEnableDesktop = true;
	bool mIsFirstCall = true;
	std::vector<ITEM> mRecentFileItems;
	SpecialFolderFiles mFiles;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(SpecialFolderFilesCommandProvider)


SpecialFolderFilesCommandProvider::SpecialFolderFilesCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mFiles.GetShortcutFiles(in->mRecentFileItems);
}

SpecialFolderFilesCommandProvider::~SpecialFolderFilesCommandProvider()
{
}

CString SpecialFolderFilesCommandProvider::GetName()
{
	return _T("SpecialFolderFiles");
}

// 一時的なコマンドを必要に応じて提供する
void SpecialFolderFilesCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		auto pref = AppPreference::Get();
		in->mIsEnableSpecialFolder = pref->IsEnableSpecialFolder();
		in->mIsEnableDesktop = pref->IsEnableDesktop();
		in->mIsFirstCall = false;
	}

	if (in->mIsEnableSpecialFolder == false && in->mIsEnableDesktop == false) {
		return ;
	}

	in->mFiles.EnableStartMenu(in->mIsEnableSpecialFolder);
	in->mFiles.EnableRecent(in->mIsEnableSpecialFolder);
	in->mFiles.EnableDesktopFiles(in->mIsEnableDesktop);
	in->mFiles.GetShortcutFiles(in->mRecentFileItems);

	for (auto& item : in->mRecentFileItems) {

		int level = pattern->Match(item.mName);
		if (level == Pattern::Mismatch) {
			continue;
		}
		auto cmd = std::make_unique<SpecialFolderFileCommand>(item);

		commands.push_back(CommandQueryItem(level, cmd.release()));
	}
}


} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

