#include "pch.h"
#include "PathExeAdhocCommandProvider.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/pathfind/ExcludePathList.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;

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
		auto pref = AppPreference::Get();
		mIsIgnoreUNC = pref->IsIgnoreUNC();
		mIsEnable = pref->IsEnablePathFind();
		mExcludeFiles.Load();
		mExeCommandPtr->Reload();
	}
	void OnAppExit() override {}

	// 環境変数PATHにあるexeを実行するためのコマンド
	PathExecuteCommand* mExeCommandPtr = nullptr;
	//
	ExcludePathList mExcludeFiles;
	//
	bool mIsIgnoreUNC = false;
	bool mIsEnable = true;
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
	bool mIsFirstCall = true;

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

	ExecuteHistory::GetInstance()->Save();
}

// コマンドの読み込み
void PathExeAdhocCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	UNREFERENCED_PARAMETER(cmdFile);

	// 内部でもつ履歴データを読み込む
	ExecuteHistory::GetInstance()->Load();
}

CString PathExeAdhocCommandProvider::GetName()
{
	return _T("PathExeAdhocCommand");
}

// 一時的なコマンドを必要に応じて提供する
void PathExeAdhocCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsIgnoreUNC = pref->IsIgnoreUNC();
		in->mIsFirstCall = false;
		in->mIsEnable = pref->IsEnablePathFind();
		in->mExcludeFiles.Load();
		in->mExeCommandPtr->Reload();
	}

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
		commands.push_back(CommandQueryItem(level, in->mExeCommandPtr));
	}

	// ToDo: HistoryCommandに責務を移動
	ExecuteHistory::ItemList items;
	ExecuteHistory::GetInstance()->GetItems(_T("pathfind"), items);
	for (auto& item : items) {
		level = pattern->Match(item.mWord);
		if (level == Pattern::Mismatch) {
			continue;
		}
		if (PathFileExists(item.mFullPath) == FALSE) {
			// 存在しないファイルは除外
			continue;
		}
		if (in->mExcludeFiles.Contains(item.mFullPath)) {
			// 除外対象のファイル
			continue;
		}
		auto cmdHist = std::make_unique<PathExecuteCommand>();
		cmdHist->SetFullPath(item.mFullPath, true);
		commands.push_back(CommandQueryItem(level, cmdHist.release()));
	}

}

} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

