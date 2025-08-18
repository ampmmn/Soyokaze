#include "pch.h"
#include "EverythingCommandProvider.h"
#include "commands/everything/EverythingAdhocCommand.h"
#include "commands/everything/EverythingCommandParam.h"
#include "commands/everything/EverythingResult.h"
#include "commands/everything/EverythingProxy.h"
#include "matcher/PatternInternal.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandRepository.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace everything {

using CommandRepository = launcherapp::core::CommandRepository;

struct EverythingCommandProvider::PImpl : 
	public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}


// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Reload();
	}
	void OnAppExit() override {}

	void Reload(); 

	CommandParam mParam;
};

void EverythingCommandProvider::PImpl::Reload()
{
	auto pref = AppPreference::Get();
	mParam.Load((Settings&)pref->GetSettings());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(EverythingCommandProvider)


EverythingCommandProvider::EverythingCommandProvider() : in(new PImpl)
{
}

EverythingCommandProvider::~EverythingCommandProvider()
{
}

CString EverythingCommandProvider::GetName()
{
	return _T("EverythingCommand");
}

// 一時的なコマンドの準備を行うための初期化
void EverythingCommandProvider::PrepareAdhocCommands()
{
	in->Reload();
}

// 一時的なコマンドを必要に応じて提供する
void EverythingCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	// 機能を利用しない場合は抜ける
	if (in->mParam.mIsEnable == false) {
		return;
	}

	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mParam.mPrefix;
	bool hasPrefix = prefix.IsEmpty() == FALSE;
	if (hasPrefix && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		return;
	}

	CString queryStr;

	// Everythingに問い合わせるための文字列を生成
 	std::vector<PatternInternal::WORD> words;
	pat2->GetWords(words);
	for (size_t i = hasPrefix ? 1 : 0; i < words.size(); ++i) {
		auto& word = words[i];
		//if (word.mMethod == PatternInternal::RegExp) {
		//	queryStr += word.mWord;
		//}
		//else {
			queryStr += word.mRawWord;
		//}
		queryStr += _T(" ");
	}

	spdlog::debug(_T("Everything QueryStr:{}"), (LPCTSTR)queryStr);

	std::vector<EverythingResult> results;
	EverythingProxy::Get()->Query(queryStr, results);

	for (auto& result : results) {

		// ファイルパス、ファイル名それぞれで一致レベルを見る
		int level = pattern->Match(result.mFullPath);
		if (level == Pattern::PartialMatch) {
			int levelFileName = pattern->Match(PathFindFileName(result.mFullPath));
			if (levelFileName >= Pattern::FrontMatch) {
				// ファイル名が完全一致/前方一致するときは少し一致レベルをよくする
				level = Pattern::FrontMatch;
			}
		}
		if (hasPrefix && level == Pattern::PartialMatch) {
			// プレフィックスありの場合は最低でも前方一致扱いにする
			level = Pattern::FrontMatch;
		}
		commands.Add(CommandQueryItem(level, new EverythingAdhocCommand(in->mParam, result)));
	}

	if (hasPrefix && results.empty()) {
			// 件数0件の場合に、弱一致の候補表示を抑制するためにダミーの項目を追加する
			commands.Add(CommandQueryItem(Pattern::HiddenMatch, new EverythingAdhocCommand()));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t EverythingCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(_T("Everything検索"));
	return 1;
}

}}} // end of namespace launcherapp::commands::everything
