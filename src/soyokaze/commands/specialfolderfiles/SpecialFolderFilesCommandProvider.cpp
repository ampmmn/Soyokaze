#include "pch.h"
#include "SpecialFolderFilesCommandProvider.h"
#include "commands/specialfolderfiles/SpecialFolderFileCommand.h"
#include "commands/specialfolderfiles/SpecialFolderFileFind.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {

using CommandRepository = launcherapp::core::CommandRepository;

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
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Reload();
	}
	void OnAppExit() override {}

	void Reload()
	{
		auto pref = AppPreference::Get();
		bool isEnable = pref->IsEnableSpecialFolder();
		mFileFind.EnableStartMenu(isEnable);
		mFileFind.EnableRecent(isEnable);
	}

	// 一覧
	SpecialFolderFileFind mFileFind;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(SpecialFolderFilesCommandProvider)


SpecialFolderFilesCommandProvider::SpecialFolderFilesCommandProvider() : in(std::make_unique<PImpl>())
{
}

SpecialFolderFilesCommandProvider::~SpecialFolderFilesCommandProvider()
{
}

CString SpecialFolderFilesCommandProvider::GetName()
{
	return _T("SpecialFolderFiles");
}

// 一時的なコマンドの準備を行うための初期化
void SpecialFolderFilesCommandProvider::PrepareAdhocCommands()
{
	in->Reload();
}

// 一時的なコマンドを必要に応じて提供する
void SpecialFolderFilesCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	std::vector<ITEM> recentFileItems;
	if (in->mFileFind.FindShortcutFiles(recentFileItems) == false) {
		return ;
	}

	for (auto& item : recentFileItems) {

		int level = pattern->Match(item.mName);
		if (level == Pattern::Mismatch) {
			continue;
		}
		auto cmd = make_refptr<SpecialFolderFileCommand>(item);

		commands.Add(CommandQueryItem(level, cmd.release()));
	}
}


} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

