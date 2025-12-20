#include "pch.h"
#include "ExplorePathCommandProvider.h"
#include "commands/explorepath/ExplorePathCommand.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/ExecutablePath.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "matcher/PartialMatchPattern.h"
#include "utility/Path.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace explorepath {

using namespace launcherapp::commands::common;

using CommandRepository = launcherapp::core::CommandRepository;

struct ExplorePathCommandProvider::PImpl : public AppPreferenceListenerIF
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
	}

	bool mIsIgnoreUNC{false};
	bool mIsEnable{true};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ExplorePathCommandProvider)


ExplorePathCommandProvider::ExplorePathCommandProvider() : in(std::make_unique<PImpl>())
{
}

ExplorePathCommandProvider::~ExplorePathCommandProvider()
{
}

CString ExplorePathCommandProvider::GetName()
{
	return _T("ExplorePathCommand");
}

// 一時的なコマンドの準備を行うための初期化
void ExplorePathCommandProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	in->Load();
}

static bool
ExtractFilePart(const CString& input, CString& output)
{
	// 先頭が二重引用符(")で始まらない場合は何もしない
	if (input[0] != _T('"')) {
		output = input;
		return true;
	}

	// 先頭が二重引用符(")で始まる場合は対応する"まで切り出す

	// (対応する"を探す)
	int len = (int)input.GetLength();
	int pos = 1;
	while (pos < len) {
		if (input[pos] != _T('"')) {
			pos++;
			continue;
		}
		break;
	}
	output = input.Mid(1, pos-1);
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void ExplorePathCommandProvider::QueryAdhocCommands(
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

	// 先頭が "..." のようにダブルクォーテーションで囲われている場合は除去
	if (len >= 2 && 
	    wholeWord[0] == _T('"') && wholeWord[len-1] == _T('"')){
		wholeWord = wholeWord.Mid(1, len-2);
	}

	// %が含まれている場合は環境変数が使われている可能性があるので展開を試みる
	if (wholeWord.Find(_T('%')) != -1) {
		DWORD sizeNeeded = ExpandEnvironmentStrings(wholeWord, nullptr, 0);
		std::vector<TCHAR> buf(sizeNeeded);
		ExpandEnvironmentStrings(wholeWord, buf.data(), sizeNeeded);
		wholeWord = buf.data();
	}

	CString filePart;
	ExtractFilePart(wholeWord, filePart);

	// 
	if (Path::FileExists(filePart)) {
		// フォルダまたはファイルのパスを示す場合
		commands.Add(CommandQueryItem(Pattern::WholeMatch, new ExplorePathCommand(filePart)));

		if (Path::IsDirectory(filePart) == false || (filePart.Right(1) != _T('\\') && filePart.Right(1) != _T('/'))) {
			return;
		}
		// パスがディレクトリ、かつ、末尾が二重引用符の場合はディレクトリ内の要素を列挙する
	}

	// 末尾の\で分けて、フォルダ内の要素を列挙する
	CString folderPath;
	CString filePattern;
	bool found = false;
	int filePartLen = filePart.GetLength();
	for (int i = filePartLen-1; i >= 0; --i) {
		auto c = filePart[i];
		if (c == _T('\\') || c == _T('/')) {
			folderPath = filePart.Left(i);
			filePattern = filePart.Mid(i+1).Trim();
			found = true;
			break;
		}
	}
	if (found == false) {
		// 区切り文字はなかった
		return ;
	}

	if (Path::IsDirectory(folderPath) == false) {
		// 区切り文字の前のパスがフォルダパスではない
		return;
	}

	RefPtr<PartialMatchPattern> patTmp(PartialMatchPattern::Create());
	patTmp->SetWholeText(filePattern);

	std::vector<ExplorePathCommand*> fileTargets;   // ファイル要素

	CString findPattern(folderPath + _T("\\*.*"));
	CFileFind f;
	BOOL isLoop = f.FindFile(findPattern, 0);

	// フォルダ以下の要素を列挙する
	while (isLoop) {
		isLoop = f.FindNextFile();
		if (f.IsDots()) {
			continue;
		}

		CString fileName = f.GetFileName();
		int level = patTmp->Match(fileName);
		if (level == Pattern::Mismatch) {
			continue;
		}

		CString filePath = f.GetFilePath();

		auto newCmd = new ExplorePathCommand(PathFindFileName(filePath), filePath);

		if (f.IsDirectory()) {
			// ディレクトリ要素を先に表示
			commands.Add(CommandQueryItem(Pattern::WholeMatch, newCmd));
		}
		else {
			// ファイルは後ろに表示するため、いったんリストに入れる
			fileTargets.push_back(newCmd);
		}
	}
	f.Close();

	// ファイル要素を後に表示
	for (auto target : fileTargets) {
			commands.Add(CommandQueryItem(Pattern::WholeMatch, target));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t ExplorePathCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(ExplorePathCommand::TypeDisplayName(false));
	displayNames.push_back(ExplorePathCommand::TypeDisplayName(true));
	return 2;
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t ExplorePathCommandProvider::GetOrder() const
{
	return 1400;
}

}}} // end of namespace launcherapp::commands::pathfind
