#include "pch.h"
#include "OutlookFolderProvider.h"
#include "commands/outlook/OutlookFolderCommand.h"
#include "commands/outlook/OutlookFolderQueryCache.h"
#include "commands/outlook/OutlookProxy.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"

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
		AppPreference::Get()->RegisterListener(this, _T("OutlookFolder"));
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
		bool isEnable = pref->IsEnableOutlookFolder();
		CString prefix = pref->GetOutlookFolderSwitchPrefix();
		if (isEnable) {
			auto proxy = OutlookProxy::GetInstance();
			proxy->Initialize();
			mQueryCache.Load(isEnable, prefix, [proxy](std::vector<QueryResult>& results) {
				return proxy->EnumFolders(results);
			});
		}
		else {
			mQueryCache.Load(isEnable, prefix, [](std::vector<QueryResult>&) {
				return false;
			});
		}
	}

	OutlookFolderQueryCache mQueryCache;
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
	auto snapshot = in->mQueryCache.GetSnapshot();

	// 機能を利用しない場合は抜ける
	if (snapshot.mIsEnable == false) {
		return;
	}
	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = snapshot.mPrefix;
	if (prefix.IsEmpty() == FALSE && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}
	// キャッシュがまだない場合はバックグラウンドでの取得完了を待つ
	if (snapshot.mHasCache == false) {
		return;
	}

	bool hasPrefix =  prefix.IsEmpty() == FALSE;
	uint32_t ignoreMask = hasPrefix ? 1 : 0;

	auto proxy = OutlookProxy::GetInstance();

	// アプリが起動していなかったらキャッシュをクリアする
	if (proxy->IsAppRunning() == false) {
		in->mQueryCache.Clear();
		return ;
	}

	int hitCount = 0;
	for (const auto& result : *snapshot.mQueryCache) {

		int level = pattern->Match(result.mFullName, ignoreMask);
		if (level == Pattern::Mismatch) {
			continue;
		}

		// プレフィックスがある場合は最低でも前方一致とする
		if (hasPrefix && level == Pattern::PartialMatch) {
			level = Pattern::FrontMatch;
		}

		commands.Add(CommandQueryItem(level, new OutlookFolderCommand(result.mFullName, result.mItemCount, result.mEntryID)));

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
