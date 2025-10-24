#include "pch.h"
#include "OutlookFolderProvider.h"
#include "commands/outlook/OutlookFolderCommand.h"
#include "commands/outlook/OutlookProxy.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace outlook {

// 一回に表示する上限数
constexpr int ITEM_LIMIT = 24;

using CommandRepository = launcherapp::core::CommandRepository;

struct OutlookFolderProvider::PImpl : 
	public AppPreferenceListenerIF
{
	PImpl() {
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl() {}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {
		AppPreference::Get()->UnregisterListener(this);
	}

	void Load() {
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableOutlookFolder();
		mPrefix = pref->GetOutlookFolderSwitchPrefix();
		if (mIsEnable) {
			auto proxy = OutlookProxy::GetInstance();
			proxy->Initialize();
		}
	}


	// キャッシュ
	std::vector<QueryResult> mQueryCache;
	// キャッシュの有無
	bool mHasCache{false};
	//
	bool mIsEnable{false};
	CString mPrefix;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(OutlookFolderProvider)


OutlookFolderProvider::OutlookFolderProvider() : in(std::make_unique<PImpl>())
{
}

OutlookFolderProvider::~OutlookFolderProvider()
{
}


CString OutlookFolderProvider::GetName()
{
	return _T("OutlookFolder");
}

// 一時的なコマンドの準備を行うための初期化
void OutlookFolderProvider::PrepareAdhocCommands()
{
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void OutlookFolderProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	// 機能を利用しない場合は抜ける
	if (in->mIsEnable == false) {
		return;
	}
	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = in->mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	bool hasPrefix =  prefix.IsEmpty() == FALSE;
	int offset = hasPrefix ? 1 : 0;

	auto proxy = OutlookProxy::GetInstance();

	if (in->mHasCache == false) {
		// キャッシュがなければ問い合わせる
		std::vector<QueryResult> queryResults;
		if (proxy->EnumFolders(queryResults) == false) {
			return ;
		}
		in->mQueryCache.swap(queryResults);
		in->mHasCache = true;
	}
	else {
		// アプリが起動してなかったらキャッシュをクリアする
		if (proxy->IsAppRunning() == false) {
			in->mQueryCache.clear();
			in->mHasCache = false;
			return ;
		}
	}

	int hitCount = 0;
	for (auto& result : in->mQueryCache) {

		int level = pattern->Match(result.mFullName, offset);
		if (level == Pattern::Mismatch) {
			continue;
		}

		// プレフィックスがある場合は最低でも前方一致とする
		if (hasPrefix && level == Pattern::PartialMatch) {
			level = Pattern::FrontMatch;
		}

		commands.Add(CommandQueryItem(level, new OutlookFolderCommand(result.mFullName, result.mItemCount, result.mFolder)));

		if (++hitCount >= ITEM_LIMIT) {
			break;
		}
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t OutlookFolderProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(OutlookFolderCommand::TypeDisplayName());
	return 1;
}

}}}
