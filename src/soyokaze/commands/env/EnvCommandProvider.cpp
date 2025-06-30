#include "pch.h"
#include "EnvCommandProvider.h"
#include "commands/env/EnvCommand.h"
#include "commands/env/EnvAppSettings.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/IFIDDefine.h"
#include "matcher/PartialMatchPattern.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace env {


using CommandRepository = launcherapp::core::CommandRepository;

struct EnvCommandProvider::PImpl:
	public AppPreferenceListenerIF
{
	PImpl()
	{
		// アプリケーションの設定変更リスナーとして登録
		AppPreference::Get()->RegisterListener(this);
	}
	~PImpl()
	{
		// アプリケーションの設定変更リスナーから解除
		AppPreference::Get()->UnregisterListener(this);
	}

	void Reload()
	{
		mSettings.Load();

		// プレフィックスが一致する場合は一覧を列挙する
		LPWCH ptr = GetEnvironmentStrings();

		std::map<CString, CString> envMap;

		auto p = ptr;
		while (p && p[0] != _T('\0')) {
			CString str(p);

			if (str[0] != _T('=')) {
				int n = 0;
				CString key = str.Tokenize(_T("="), n);
				if (key.IsEmpty() == FALSE) {
					CString value = str.Mid(n);
					envMap[key] = value;
				}
			}

			p = _tcschr(p, _T('\0'));
			if (p == nullptr) {
				break;
			}
			p++;
		}

		if (ptr) {
			FreeEnvironmentStrings(ptr);
		}

		mEnvMap.swap(envMap);
		mIsLoaded = true;
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Reload();
	}
	void OnAppExit() override {}

	AppSettings mSettings;
	std::map<CString,CString> mEnvMap;
	bool mIsLoaded{false};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(EnvCommandProvider)


EnvCommandProvider::EnvCommandProvider() : in(std::make_unique<PImpl>())
{
}

EnvCommandProvider::~EnvCommandProvider()
{
}

CString EnvCommandProvider::GetName()
{
	return _T("Env");
}

// 一時的なコマンドを必要に応じて提供する
void EnvCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	CString cmdline = pattern->GetWholeString();
	cmdline = cmdline.Trim();
	if (cmdline.IsEmpty()) {
		// キーワードが空なら何もしない
		return;
	}

	const auto& prefix = in->mSettings.mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) == 0) {
		// プレフィックスが一致するケース

		if (in->mIsLoaded == false) {
			// 初回はロード
			in->Reload();
		}

		// 機能を利用しない場合は抜ける
		if (in->mSettings.mIsEnable == false) {
			return ;
		}

		RefPtr<PatternInternal> pat2;
		if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
			return ;
		}

		std::vector<CString> words;
		CString queryStr;
		pat2->GetRawWords(words);
		for (size_t i = 1; i < words.size(); ++i) {
			queryStr += words[i];
			queryStr += _T(" ");
		}

		// (prfix)の後に入力したキーワードのみで別のPartialMatchPatternを生成しておく
		RefPtr<PartialMatchPattern> patTmp(PartialMatchPattern::Create());
		patTmp->SetWholeText(queryStr);

		bool isMatched = false;
		for (auto item : in->mEnvMap) {
			auto& key = item.first;
			auto& value = item.second;

			int level = patTmp->Match(key);
			if (level == Pattern::Mismatch) {
				continue;
			}

			if (level == Pattern::PartialMatch) {
				// プレフィックスが一致しているので最低でも前方一致とする
				level = Pattern::FrontMatch;
			}
			commands.Add(CommandQueryItem(level, new EnvCommand(key, value)));
			isMatched = true;
		}
		if (isMatched == false) {
			// 件数0件の場合でも、弱一致の候補表示を抑制するためにダミーの項目を追加する
			commands.Add(CommandQueryItem(Pattern::HiddenMatch, new EnvCommand(_T(""), _T(""))));
			return;
		}

	}
	else if (cmdline.GetLength() >= 3 && cmdline[0] == _T('%') && cmdline[cmdline.GetLength()-1] == _T('%')) {
		// %...% の場合は該当する変数を表示

		CString valName = cmdline.Mid(1, cmdline.GetLength()-2);

		size_t reqLen = 0;
		if (_tgetenv_s(&reqLen, nullptr, 0, valName) != 0) {
			return ;
		}
		if (reqLen == 0) {
			return;
		}

		CString value;
		_tgetenv_s(&reqLen, value.GetBuffer((int)reqLen), reqLen, valName);
		value.ReleaseBuffer();

		commands.Add(CommandQueryItem(Pattern::WholeMatch, new EnvCommand(valName, value)));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t EnvCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(EnvCommand::TypeDisplayName());
	return 0;
}


} // end of namespace env
} // end of namespace commands
} // end of namespace launcherapp

