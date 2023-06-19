#include "pch.h"
#include "HistoryCommandProvider.h"
#include "commands/history/HistoryCommand.h"
#include "commands/common/ExecuteHistory.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ExecuteHistory = soyokaze::commands::common::ExecuteHistory;

namespace soyokaze {
namespace commands {
namespace history {


using CommandRepository = soyokaze::core::CommandRepository;

struct HistoryCommandProvider::PImpl
{
	uint32_t mRefCount;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(HistoryCommandProvider)


HistoryCommandProvider::HistoryCommandProvider() : in(new PImpl)
{
	in->mRefCount = 1;
}

HistoryCommandProvider::~HistoryCommandProvider()
{
}

// 初回起動の初期化を行う
void HistoryCommandProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void HistoryCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// サポートしない
}

CString HistoryCommandProvider::GetName()
{
	return _T("HistoryCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString HistoryCommandProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString HistoryCommandProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool HistoryCommandProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool HistoryCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void HistoryCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	std::vector<CommandQueryItem>& commands
)
{
	ExecuteHistory::ItemList items;
	ExecuteHistory::GetInstance()->GetItems(_T("history"), items);

	for (const auto& item : items) {

		int level = pattern->Match(item.mWord);
		if (level != Pattern::Mismatch) {
			auto command = new HistoryCommand(item.mWord);
			commands.push_back(CommandQueryItem(level, command));
		}
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t HistoryCommandProvider::HistoryCommandProvider::GetOrder() const
{
	return 1000;
}

uint32_t HistoryCommandProvider::HistoryCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t HistoryCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


} // end of namespace history
} // end of namespace commands
} // end of namespace soyokaze

