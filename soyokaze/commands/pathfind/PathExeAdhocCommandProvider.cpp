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

using ExecuteHistory = soyokaze::commands::common::ExecuteHistory;

namespace soyokaze {
namespace commands {
namespace pathfind {


using CommandRepository = soyokaze::core::CommandRepository;

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
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsIgnoreUNC = pref->IsIgnoreUNC();
		mExcludeFiles.Load();
		mExeCommandPtr->Reload();
	}
	void OnAppExit() override {}

	// 環境変数PATHにあるexeを実行するためのコマンド
	PathExecuteCommand* mExeCommandPtr;
	//
	ExcludePathList mExcludeFiles;
	//
	bool mIsIgnoreUNC;
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
	bool mIsFirstCall;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PathExeAdhocCommandProvider)


PathExeAdhocCommandProvider::PathExeAdhocCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mExeCommandPtr = new PathExecuteCommand(&in->mExcludeFiles);
	in->mIsIgnoreUNC = false;
	in->mIsFirstCall = true;
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
		in->mExcludeFiles.Load();
		in->mExeCommandPtr->Reload();
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
} // end of namespace soyokaze

