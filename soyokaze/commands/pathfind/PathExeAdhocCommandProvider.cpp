#include "pch.h"
#include "PathExeAdhocCommandProvider.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/pathfind/ExecuteHistory.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace pathfind {


using CommandRepository = soyokaze::core::CommandRepository;

struct PathExeAdhocCommandProvider::PImpl
{
	uint32_t mRefCount;
	// 環境変数PATHにあるexeを実行するためのコマンド
	PathExecuteCommand* mExeCommandPtr;
	// 実行履歴を保持する
	ExecuteHistory mHistory;

	// 初回呼び出しフラグ(初回呼び出し時に履歴をロードするため)
	bool mIsFirstCall;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PathExeAdhocCommandProvider)


PathExeAdhocCommandProvider::PathExeAdhocCommandProvider() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mExeCommandPtr = new PathExecuteCommand();
	in->mExeCommandPtr->SetHistoryList(&in->mHistory);
	in->mIsFirstCall = true;
}

PathExeAdhocCommandProvider::~PathExeAdhocCommandProvider()
{
	if (in->mExeCommandPtr) {
		in->mExeCommandPtr->Release();
	}

	in->mHistory.Save();
}

// 初回起動の初期化を行う
void PathExeAdhocCommandProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void PathExeAdhocCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// サポートしない
}

CString PathExeAdhocCommandProvider::GetName()
{
	return _T("PathExeAdhocCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString PathExeAdhocCommandProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString PathExeAdhocCommandProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool PathExeAdhocCommandProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool PathExeAdhocCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void PathExeAdhocCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	int level = in->mExeCommandPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mExeCommandPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mExeCommandPtr));
	}

	// 初回問い合わせ時に実行履歴情報をロードする
	if (in->mIsFirstCall) {
		in->mIsFirstCall = false;
		in->mHistory.Load();
	}

	std::vector<HISTORY_ITEM> items;
	in->mHistory.GetItems(items);
	for (auto& item : items) {
		level = pattern->Match(item.mWord);
		if (level == Pattern::Mismatch) {
			continue;
		}
		auto cmdHist = new PathExecuteCommand();
		cmdHist->SetFullPath(item.mFullPath);
		cmdHist->SetHistoryList(&in->mHistory);
		commands.push_back(CommandQueryItem(level, cmdHist));
	}

}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t PathExeAdhocCommandProvider::PathExeAdhocCommandProvider::GetOrder() const
{
	return 1000;
}

uint32_t PathExeAdhocCommandProvider::PathExeAdhocCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t PathExeAdhocCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze

