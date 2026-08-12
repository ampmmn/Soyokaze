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
		// 従来のVSCode履歴情報の保存先ファイルパス
		Path dbPath1(Path::APPDATA, _T("code\\user\\globalStorage\\state.vscdb"));
		// ある時点から下記のファイルに設定情報が保存されるようになっている
		Path dbPath2(Path::USERPROFILE, _T(".vscode-shared\\sharedStorage\\state.vscdb"));

		if (dbPath1.FileExists() == false && dbPath2.FileExists() == false) {
			// どちらも存在しない場合、VSCodeはおそらくインストールされていない
			return false;
		}

		if (mParam.mIsEnable == false) {
			// 機能が無効ならファイル変更通知を解除し、なにもしない
			if (mWatchId1 != 0) {
				LocalDirectoryWatcher::GetInstance()->Unregister(mWatchId1);
				mWatchId1 = 0;
			}
			if (mWatchId2 != 0) {
				LocalDirectoryWatcher::GetInstance()->Unregister(mWatchId2);
				mWatchId2 = 0;
			}
			return false;
		}

		// ファイルの更新があったら通知を受け取るための登録をする
		if (mWatchId1 == 0 || mWatchId2 == 0) {
			// オリジナルの履歴データベースファイルが更新されたら通知をもらうための登録をする
			auto callback = [](void* p) {
				auto thisPtr = (PImpl*)p;
				// 更新があったときもリロード
				thisPtr->LoadStorage();
			};
			if (mWatchId1 == 0) {
				mWatchId1 = LocalDirectoryWatcher::GetInstance()->Register(dbPath1, callback, this);
			}
			if (mWatchId2 == 0) {
				mWatchId2 = LocalDirectoryWatcher::GetInstance()->Register(dbPath2, callback, this);
			}
		}

		std::vector<RefPtr<launcherapp::core::Command> > commands;
		LoadStorage(dbPath1, commands);
		LoadStorage(dbPath2, commands);

		std::lock_guard<std::mutex> lock(mMutex);
		mCommands.swap(commands);

		return true;
	}

	bool LoadStorage(LPCTSTR dbFilePath, std::vector<RefPtr<launcherapp::core::Command> >& commands)
	{
		if (Path::FileExists(dbFilePath) == false) {
			return false;
		}

		// state.vscdbから履歴情報を取得する
		try {
			SQLite3Database db(dbFilePath, true);
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
			return true;
		} catch(const json::exception& e) {
			CString what;
			UTF2UTF(e.what(), what);
			spdlog::warn(_T("failed to parse state.vcsdb:{0}, {1}"), dbFilePath, (LPCTSTR)what);
			return false;
		}
		return false;
	}

	CommandParam mParam;
	std::vector<RefPtr<launcherapp::core::Command> > mCommands;
	uint32_t mWatchId1{0};
	uint32_t mWatchId2{0};
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
	if (in->mWatchId1 != 0) {
		LocalDirectoryWatcher::GetInstance()->Unregister(in->mWatchId1);
		in->mWatchId1 = 0;
	}
	if (in->mWatchId2 != 0) {
		LocalDirectoryWatcher::GetInstance()->Unregister(in->mWatchId2);
		in->mWatchId2 = 0;
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

