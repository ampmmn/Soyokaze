#include "pch.h"
#include "PathExeAdhocCommandProvider.h"
#include "commands/pathfind/PathExecuteCommand.h"
#include "commands/pathfind/PathURLCommand.h"
#include "commands/pathfind/ExcludePathList.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "utility/LocalPathResolver.h"
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
using LocalPathResolver = launcherapp::utility::LocalPathResolver;

static CString EXE_EXT = _T(".exe");

static const tregex& GetURLRegex()
{
	static tregex reg(_T("https?://.+"));
	return reg;
}


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
		mResolver.ResetPath();

		// 追加パスを登録
		std::vector<CString> paths;
		pref->GetAdditionalPaths(paths);

		for (auto& path : paths) {
			mResolver.AddPath(path);
		}
	}

	ExcludePathList mExcludeFiles;
	LocalPathResolver mResolver;
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
}

PathExeAdhocCommandProvider::~PathExeAdhocCommandProvider()
{
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

	CString wholeWord = pattern->GetWholeString();
	wholeWord.Trim();

	int len = wholeWord.GetLength();

	// 先頭が "..." で囲われている場合は除去
	if (len >= 2 && 
	    wholeWord[0] == _T('"') && wholeWord[len-1] == _T('"')){
		wholeWord = wholeWord.Mid(1, len-2);
	}

	// URLパターンマッチするかを判定
	const tregex& regURL = GetURLRegex();
	if (std::regex_search((LPCTSTR)wholeWord, regURL)) {
		commands.Add(CommandQueryItem(Pattern::WholeMatch, new PathURLCommand(wholeWord)));
		return ;
	}

	// %が含まれている場合は環境変数が使われている可能性があるので展開を試みる
	if (wholeWord.Find(_T('%')) != -1) {
		DWORD sizeNeeded = ExpandEnvironmentStrings(wholeWord, nullptr, 0);
		std::vector<TCHAR> buf(sizeNeeded);
		ExpandEnvironmentStrings(wholeWord, buf.data(), sizeNeeded);
		wholeWord = buf.data();
	}

	CString filePart;

	int pos = 0;
	// "で始まる場合は対応する"まで切り出す
	if (wholeWord[pos] == _T('"')) {
		pos++;

		// 対応する"を探す
		while (pos < len) {
			if (wholeWord[pos] != _T('"')) {
				pos++;
				continue;
			}
			break;
		}
		if (pos == len) {
			// 対応する"がなかった
			return;
		}
		filePart = wholeWord.Mid(1, pos-1);
	}
	else {
		filePart = wholeWord;
	}

	// 絶対パス指定、かつ、存在するパスの場合は候補として表示
	if (PathIsRelative(filePart) == FALSE && Path::FileExists(filePart)) {
		commands.Add(CommandQueryItem(Pattern::WholeMatch, new PathExecuteCommand(filePart)));
		return ;
	}

	CString word = pattern->GetFirstWord();
	if (EXE_EXT.CompareNoCase(PathFindExtension(word)) != 0) {
		word += _T(".exe");
	}

	if (PathIsRelative(word) == FALSE) {
		return;
	}

	// 相対パスを解決する
	CString resolvedPath;
	if (in->mResolver.Resolve(word, resolvedPath)) {

		// 除外対象に含まれなければ
		if (in->mExcludeFiles.Contains(resolvedPath) == false) {
			commands.Add(CommandQueryItem(Pattern::WholeMatch, new PathExecuteCommand(word, resolvedPath)));
		}
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t PathExeAdhocCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(PathExecuteCommand::TypeDisplayName());
	return 1;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t PathExeAdhocCommandProvider::GetOrder() const
{
	return 1400;
}

} // end of namespace pathfind
} // end of namespace commands
} // end of namespace launcherapp

