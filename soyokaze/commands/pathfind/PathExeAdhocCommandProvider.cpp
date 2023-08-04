#include "pch.h"
#include "PathExeAdhocCommandProvider.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"
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
	}
	void OnAppExit() override {}

	uint32_t mRefCount;
	// 環境変数PATHにあるexeを実行するためのコマンド
	PathExecuteCommand* mExeCommandPtr;
	//
	bool mIsIgnoreUNC;
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
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
	// 内部でもつ履歴データを読み込む
	ExecuteHistory::GetInstance()->Load();
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
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsIgnoreUNC = pref->IsIgnoreUNC();
		in->mIsFirstCall = false;
	}

	if (in->mIsIgnoreUNC) {
		CString word = pattern->GetOriginalPattern();
		if (PathIsUNC(word)) {
			// ネットワークパスを無視する
			return ;
		}
	}

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
		auto cmdHist = new PathExecuteCommand();
		cmdHist->SetFullPath(item.mFullPath, true);
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

