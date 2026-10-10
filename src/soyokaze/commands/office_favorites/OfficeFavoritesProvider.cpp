#include "pch.h"
#include "OfficeFavoritesProvider.h"
#include "OfficeFavoritesCommand.h"
#include "OfficeFavorites.h"
#include "utility/RefPtr.h"
#include "utility/RegistryKey.h"
#include "utility/LocalDirectoryWatcher.h"
#include "commands/core/CommandRepository.h"
#include "app/LauncherEventDispatcher.h"
#include "core/LauncherEventListenerIF.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace office_favorites {

namespace {

// 変更通知用イベントのハンドル(RAII)
using AutoEvent = std::unique_ptr<void, decltype(&::CloseHandle)>;

// レジストリ監視の単位(User MRU キー1つ分)
struct RegistryWatch
{
	// 監視対象のキー
	RegistryKey mKey;
	// 変更通知を受け取るイベント
	AutoEvent mEvent;
};

// 監視する変更の種類(サブキーの追加/削除と値の変更)
const DWORD REGISTRY_NOTIFY_FLAGS = REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET;

// JSON更新通知を受けてから読み込むまでの待機時間(書き込み途中を読まないため)
const DWORD JSON_RELOAD_DELAY_MSEC = 250;

/**
 * @brief RegNotifyChangeKeyValue を登録する
 * @note 通知は一度きりで解除されるため、通知を受けるたびに再登録する
 * @param[in,out] watch 監視対象
 * @return true:登録成功  false:登録失敗
 */
bool ArmRegistryWatch(RegistryWatch& watch)
{
	// サブツリーも含めて監視する
	LONG ret = RegNotifyChangeKeyValue(watch.mKey.GetHandle(), TRUE, REGISTRY_NOTIFY_FLAGS, watch.mEvent.get(), TRUE);
	return ret == ERROR_SUCCESS;
}

} // end of unnamed namespace

struct OfficeFavoritesProvider::PImpl : 
	public AppPreferenceListenerIF,
	public LauncherEventListenerIF
{
	PImpl() : mFavorites(std::make_unique<OfficeFavorites>())
	{
		AppPreference::Get()->RegisterListener(this, _T("OfficeFavorites"));
		LauncherEventDispatcher::Get()->AddListener(this);
	}
	virtual ~PImpl()
	{
		LauncherEventDispatcher::Get()->RemoveListener(this);
		AppPreference::Get()->UnregisterListener(this);

		// 監視を止めてから保持データを破棄する
		Stop();
	}

	// 有効/無効に応じて開始・停止する
	void SetEnable(bool isEnable)
	{
		if (isEnable) {
			Start();
		}
		else {
			Stop();
		}
	}

	// 情報の読み込みと監視の開始(既に開始している場合は何もしない)
	void Start()
	{
		if (mIsEnable) {
			return;
		}
		mIsEnable = true;

		mFavorites->ReloadRegistry();
		mFavorites->ReloadJson();
		StartRegistryWatch();
		StartJsonWatch();
	}

	// 監視の停止と保持データの破棄
	void Stop()
	{
		mIsEnable = false;

		StopJsonWatch();
		mRegistryWatches.clear();
		mFavorites->Clear();
	}

	// レジストリの変更監視を開始する
	void StartRegistryWatch();
	// JSONファイルの変更監視を開始する
	void StartJsonWatch();
	// JSONファイルの変更監視を停止する
	void StopJsonWatch();

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		SetEnable(AppPreference::Get()->IsEnableOfficeFavorites());
	}
	void OnAppExit() override {}

// LauncherEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	// 一定周期で呼ばれるので、レジストリの変更通知を確認する
	void OnTimer() override;
	void OnLauncherActivate() override {}
	void OnLauncherUnactivate() override {}
	void OnMonitorConfigurationChanged() override {}

	bool mIsEnable{false};

	// お気に入りの保持
	std::unique_ptr<OfficeFavorites> mFavorites;
	// レジストリの監視一覧
	std::vector<RegistryWatch> mRegistryWatches;
	// JSONファイルの監視ID(LocalDirectoryWatcherの登録ID)
	std::vector<uint32_t> mJsonWatchIds;
};

// レジストリの変更監視を開始する
void OfficeFavoritesProvider::PImpl::StartRegistryWatch()
{
	mRegistryWatches.clear();

	std::vector<CString> keyPaths;
	OfficeFavorites::GetUserMRUKeyPaths(keyPaths);

	RegistryKey HKCU(HKEY_CURRENT_USER);
	for (auto& keyPath : keyPaths) {
		// Officeが未導入などでキーが存在しない場合は対象外
		RegistryKey key;
		if (HKCU.OpenSubKey(keyPath, key) == false) {
			continue;
		}

		// 監視キーごとに独立したイベントを用意する
		HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
		if (hEvent == nullptr) {
			spdlog::warn(_T("failed to create event for office favorites registry."));
			continue;
		}

		RegistryWatch watch{ std::move(key), AutoEvent(hEvent, &::CloseHandle) };
		if (ArmRegistryWatch(watch) == false) {
			spdlog::warn(_T("failed to register registry watch. {0}"), (LPCTSTR)keyPath);
			continue;
		}
		mRegistryWatches.push_back(std::move(watch));
	}
}

// JSONファイルの変更監視を開始する
void OfficeFavoritesProvider::PImpl::StartJsonWatch()
{
	StopJsonWatch();

	auto watcher = LocalDirectoryWatcher::GetInstance();

	std::vector<CString> files;
	OfficeFavorites::EnumAggMruJsonFiles(files);
	for (auto& file : files) {
		// ファイルを指定すると、そのファイルが置かれたディレクトリを監視する
		uint32_t id = watcher->Register((LPCTSTR)file, [](void* p) {
				spdlog::info("office favorites json updated.");
				// 通知を受けて即ロードするとファイルアクセスに失敗するので少し間を置く
				Sleep(JSON_RELOAD_DELAY_MSEC);
				auto favorites = static_cast<OfficeFavorites*>(p);
				favorites->ReloadJson();
			}, mFavorites.get());
		if (id != 0) {
			mJsonWatchIds.push_back(id);
		}
	}
}

// JSONファイルの変更監視を停止する
void OfficeFavoritesProvider::PImpl::StopJsonWatch()
{
	auto watcher = LocalDirectoryWatcher::GetInstance();
	for (auto id : mJsonWatchIds) {
		watcher->Unregister(id);
	}
	mJsonWatchIds.clear();
}

// レジストリの変更通知を確認し、変更があれば読み直す
void OfficeFavoritesProvider::PImpl::OnTimer()
{
	if (mIsEnable == false) {
		return;
	}

	bool isChanged = false;
	for (auto& watch : mRegistryWatches) {
		// 非ブロッキングで通知の有無を確認する
		if (WaitForSingleObject(watch.mEvent.get(), 0) != WAIT_OBJECT_0) {
			continue;
		}

		// 通知を消費してから再登録する(再登録後に読み直すことで変更を取りこぼさない)
		ResetEvent(watch.mEvent.get());
		if (ArmRegistryWatch(watch) == false) {
			spdlog::warn(_T("failed to re-register registry watch."));
		}
		isChanged = true;
	}

	if (isChanged) {
		mFavorites->ReloadRegistry();
	}
}

REGISTER_COMMANDPROVIDER(OfficeFavoritesProvider)

OfficeFavoritesProvider::OfficeFavoritesProvider() : in(std::make_unique<PImpl>())
{
}

OfficeFavoritesProvider::~OfficeFavoritesProvider()
{
}

CString OfficeFavoritesProvider::GetName()
{
	return _T("OfficeFavoritesCommand");
}

// 一時的なコマンドの準備を行うための初期化
void OfficeFavoritesProvider::PrepareAdhocCommands()
{
	// 設定を読み込み、有効であれば情報の読み込みと監視を開始する
	auto pref = AppPreference::Get();
	in->SetEnable(pref->IsEnableOfficeFavorites());
}

// 一時的なコマンドを必要に応じて提供する
void OfficeFavoritesProvider::QueryAdhocCommands(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	auto items = in->mFavorites->GetItems();
	for (auto& item : items) {
		// 名前で絞り込む
		int level = pattern->Match(item.GetName());
		if (level == Pattern::Mismatch) {
			continue;
		}

		auto cmd = make_refptr<OfficeFavoritesCommand>(item);
		commands.Add(CommandQueryItem(level, cmd.release()));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t OfficeFavoritesProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(OfficeFavoritesCommand::TypeDisplayName(_T("Excel")));
	displayNames.push_back(OfficeFavoritesCommand::TypeDisplayName(_T("Word")));
	displayNames.push_back(OfficeFavoritesCommand::TypeDisplayName(_T("PowerPoint")));
	return 3;
}

} // end of namespace office_favorites
} // end of namespace commands
} // end of namespace launcherapp
