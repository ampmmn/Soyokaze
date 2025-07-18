#include "pch.h"
#include "PathExeAdhocCommandProvider.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/pathfind/ExcludePathList.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "utility/Path.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace pathfind {


using CommandRepository = launcherapp::core::CommandRepository;

struct PathExeAdhocCommandProvider::PImpl : public AppPreferenceListenerIF
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
		mIsIgnoreUNC = pref->IsIgnoreUNC();
		mIsEnable = pref->IsEnablePathFind();
		mExcludeFiles.Load();
		mExeCommandPtr->Reload();
	}

	// 環境変数PATHにあるexeを実行するためのコマンド
	PathExecuteCommand* mExeCommandPtr{nullptr};
	//
	ExcludePathList mExcludeFiles;
	//
	bool mIsIgnoreUNC{false};
	bool mIsEnable{true};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PathExeAdhocCommandProvider)


PathExeAdhocCommandProvider::PathExeAdhocCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mExeCommandPtr = new PathExecuteCommand(&in->mExcludeFiles);
}

PathExeAdhocCommandProvider::~PathExeAdhocCommandProvider()
{
	if (in->mExeCommandPtr) {
		in->mExeCommandPtr->Release();
	}
}

// コマンドの読み込み
void PathExeAdhocCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	UNREFERENCED_PARAMETER(cmdFile);
}

CString PathExeAdhocCommandProvider::GetName()
{
	return _T("PathExeAdhocCommand");
}

// 一時的なコマンドの準備を行うための初期化
void PathExeAdhocCommandProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void PathExeAdhocCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsEnable == false) {
		// 機能は無効化されている
		return ;
	}

	if (in->mIsIgnoreUNC) {
		CString word = pattern->GetWholeString();
		if (PathIsUNC(word)) {
			// ネットワークパスを無視する
			return ;
		}
	}

	// 見つかったパスを実行するコマンド
	int level = in->mExeCommandPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mExeCommandPtr->AddRef();
		commands.Add(CommandQueryItem(level, in->mExeCommandPtr));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t PathExeAdhocCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(PathExecuteCommand::TypeDisplayName());
	return 1;
}

} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

