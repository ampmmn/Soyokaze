#include "pch.h"
#include "VSCodeCommand.h"
#include "commands/vscode/VSCodeCommandParam.h"
#include "commands/vscode/VSCodeFileCommand.h"
#include "commands/vscode/VSCodeFolderCommand.h"
#include "commands/vscode/VSCodeFileCommand.h"
#include "commands/vscode/VSCodeWorkspaceCommand.h"
#include "utility/Path.h"
#include "utility/SQLite3Database.h"
#include "utility/LocalDirectoryWatcher.h"
#include "setting/AppPreference.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace vscode {

using SQLite3Database = launcherapp::utility::SQLite3Database; 

struct VSCodeCommand::PImpl
{
	bool LoadStorage()
	{
		Path dbPath(Path::APPDATA, _T("code/user/globalStorage/state.vscdb"));
		if (dbPath.FileExists() == false) {
			// VSCodeはおそらくインストールされていない
			return false;
		}

		if (mParam.mIsEnable == false) {
			if (mWatchId != 0) {
				LocalDirectoryWatcher::GetInstance()->Unregister(mWatchId);
				mWatchId = 0;
			}
			return false;
		}

		// 更新があったら通知を受け取るための登録
		if (mWatchId == 0) {
			// オリジナルの履歴データベースファイルが更新されたら通知をもらうための登録をする
			mWatchId = LocalDirectoryWatcher::GetInstance()->Register(dbPath, [](void* p) {
				auto thisPtr = (PImpl*)p;
				// 更新があったときもリロード
				thisPtr->LoadStorage();
			}, this);
		}


		// state.vscdbから履歴情報を取得する
		try {
			SQLite3Database db((LPCTSTR)dbPath, true);
			LPCTSTR query =_T("SELECT value FROM ItemTable WHERE key = 'history.recentlyOpenedPathsList';"); 

			std::string jsonStr;
			db.Query(query, [](void*p,int, char** argv, char**) -> int {
					std::string* entriesJSON = (std::string*)p;
					*entriesJSON = argv[0];
					return 0;
			}, &jsonStr);

// JSONは以下の構造を持つ
// {
// "entries":[
// 	{"workspace":{"id":"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx","configPath":"file:///path/to/name.code-workspace"}},
//   {"folderUri":"file:///path/to/folder"},
// 	{"fileUri":"file:///path/to/file"}
// ]
// }

		// 上記のJSONを解析してデータを保持する
			std::vector<RefPtr<launcherapp::core::Command> > commands;
			json jsonObj = json::parse(jsonStr);
			auto entries = jsonObj["entries"];
			for (auto it = entries.begin(); it != entries.end(); ++it) {

				auto entry = it.value(); 

				RefPtr<launcherapp::core::Command> cmd;
				if (VSCodeWorkspaceCommand::Create(entry, &mParam, &cmd)) {
					commands.push_back(cmd);
				}
				else if (VSCodeFolderCommand::Create(entry, &mParam, &cmd)) {
					commands.push_back(cmd);
				}
				else if (VSCodeFileCommand::Create(entry, &mParam, &cmd)) {
					commands.push_back(cmd);
				}
			}

			std::lock_guard<std::mutex> lock(mMutex);
			mCommands.swap(commands);
			int n = 0;
		}
		catch(std::exception&) {
			spdlog::warn(_T("failed to parse state.vcsdb:{}"), (LPCTSTR)dbPath);
			return false;
		}

		return true;
	}

	CommandParam mParam;
	std::vector<RefPtr<launcherapp::core::Command> > mCommands;
	uint32_t mWatchId{0};
	std::mutex mMutex;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



VSCodeCommand::VSCodeCommand() : in(std::make_unique<PImpl>())
{
}

VSCodeCommand::~VSCodeCommand()
{
	if (in->mWatchId != 0) {
		LocalDirectoryWatcher::GetInstance()->Unregister(in->mWatchId);
		in->mWatchId = 0;
	}
}

bool VSCodeCommand::Load()
{
	auto pref = AppPreference::Get();
	in->mParam.Load((Settings&)pref->GetSettings());

	in->LoadStorage();

	return true;
}

/**
 	コマンドの候補として追加表示する項目を取得する
 	@return true:取得成功   false:取得失敗(表示しない)
 	@param[in]  pattern  入力パターン
 	@param[out] commands 表示する候補
*/
bool VSCodeCommand::QueryCandidates(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	// 機能を利用しない場合は抜ける
	if (in->mParam.mIsEnable == false) {
		return false;
	}

	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mParam.mPrefix;
	bool hasPrefix = prefix.IsEmpty() == FALSE;
	if (hasPrefix && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return false;
	}

	// 問い合わせ文字列の長さが閾値を下回る場合は機能を発動しない
	if (_tcslen(pattern->GetWholeString()) < in->mParam.mMinTriggerLength) {
		return false;
	}

	std::lock_guard<std::mutex> lock(in->mMutex);
	for (auto& cmd : in->mCommands) {
		int level = cmd->Match(pattern);
		if (level == Pattern::Mismatch) {
			continue;
		}
		if (level == Pattern::PartialMatch && in->mParam.HasPrefix()) {
			level = Pattern::FrontMatch;
		}

		cmd->AddRef();
		commands.Add(CommandQueryItem(level, cmd.get()));
	}

	return true;
}

uint32_t VSCodeCommand::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(VSCodeWorkspaceCommand::TypeDisplayName());
	displayNames.push_back(VSCodeFolderCommand::TypeDisplayName());
	displayNames.push_back(VSCodeFileCommand::TypeDisplayName());
	return 3;
}

}}} // end of namespace launcherapp::commands::webhistory

