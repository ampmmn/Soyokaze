#include "pch.h"
#include "OneNoteCommandProvider.h"
#include "commands/onenote/OneNoteCommandParam.h"
#include "commands/onenote/OneNoteCommand.h"
#include "commands/onenote/OneNoteAppProxy.h"
#include "commands/onenote/OneNoteSection.h"
#include "commands/onenote/OneNotePage.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "core/LauncherEventListenerIF.h"
#include "app/LauncherEventDispatcher.h"
#include <atomic>
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp { namespace commands { namespace onenote {

constexpr int SEARCH_LIMIT = 24;
const uint64_t UPDATE_INTERVAL = 5 * 60 * 1000;  // 5分

struct OneNoteCommandProvider::PImpl : 
	public AppPreferenceListenerIF,
	public LauncherEventListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this, _T("OneNote"));
		LauncherEventDispatcher::Get()->AddListener(this);
	}
	virtual ~PImpl()
	{
		mIsStopping = true;
		LauncherEventDispatcher::Get()->RemoveListener(this);
		AppPreference::Get()->UnregisterListener(this);
		WaitForLoad();
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {}

// LauncherEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {

		CommandParam param = GetParam();
		if (param.mIsEnable == false) {
			// 機能を利用しない
			return;
		}

		// 一定間隔で更新を監視し、更新を検知したら再読み込み
		if (mIsBackground == false) {
			// 前面にあるときはリロードしない
			return ;
		}


		if (mLastLoadTime != 0 && GetTickCount64() - mLastLoadTime <= UPDATE_INTERVAL) {
			// 前回の読み込みから一定時間が経過しないうちは再読み込みしない
			return;
		}

		StartLoad();
	}

	void OnLauncherActivate() override
	{
		mIsBackground = false;
	}
	void OnLauncherUnactivate() override
	{
		mIsBackground = true;
	}
	void OnMonitorConfigurationChanged() override {}


	void Load();
	/**
	  設定をコピーして取得する
	  @return 設定情報
	*/
	CommandParam GetParam()
	{
		std::lock_guard<std::mutex> lock(mParamMutex);
		return mParam;
	}
	/**
	  OneNoteのノートブック一覧をバックグラウンドで読み込む
	*/
	void StartLoad()
	{
		if (mIsStopping) {
			return;
		}
		if (mIsLoading.exchange(true)) {
			return;
		}

		std::lock_guard<std::mutex> lock(mLoadThreadMutex);
		if (mIsStopping) {
			mIsLoading = false;
			return;
		}
		if (mLoadThread.joinable()) {
			mLoadThread.join();
		}

		// OneNoteへの接続を検索スレッドから分離する
		spdlog::info("Reload Onenote Notebook.");
		mLoadThread = std::thread([this]() {
			HRESULT hr = CoInitialize(nullptr);
			if (SUCCEEDED(hr)) {
				LoadNotebooks();
				CoUninitialize();
			}
			mIsLoading = false;
		});
	}
	/**
	  ノートブック読み込みスレッドの終了を待つ
	*/
	void WaitForLoad()
	{
		std::lock_guard<std::mutex> lock(mLoadThreadMutex);
		if (mLoadThread.joinable()) {
			mLoadThread.join();
		}
	}
	void LoadNotebooks() {
		std::vector<OneNoteBook> books;
		OneNoteAppProxy app;
		if (app.GetHierarchy(books) == false) {
			return;
		}

		std::lock_guard<std::mutex> lock(mMutex); 
		std::swap(mBooks, books);
		mLastLoadTime = GetTickCount64();
		mIsReady = true;
	}

	CommandParam mParam;
	std::mutex mParamMutex;
	std::vector<OneNoteBook> mBooks;
	std::mutex mMutex;
	bool mIsReady{false};
	bool mIsBackground{false};
	std::atomic<uint64_t> mLastLoadTime{0};
	std::atomic<bool> mIsLoading{false};
	std::atomic<bool> mIsStopping{false};
	std::mutex mLoadThreadMutex;
	// ブックマーク読み込みスレッド
	std::thread mLoadThread;
};

/**
	レジストリを参照し、保存されたセッション名の一覧を取得する
*/
void OneNoteCommandProvider::PImpl::Load()
{
	auto pref = AppPreference::Get();
	std::lock_guard<std::mutex> lock(mParamMutex);
	mParam.Load((Settings&)pref->GetSettings());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(OneNoteCommandProvider)


OneNoteCommandProvider::OneNoteCommandProvider() : in(std::make_unique<PImpl>())
{
}

OneNoteCommandProvider::~OneNoteCommandProvider() = default;

CString OneNoteCommandProvider::GetName()
{
	return _T("OneNote");
}

// 一時的なコマンドの準備を行うための初期化
void OneNoteCommandProvider::PrepareAdhocCommands()
{
	// 設定情報を読む
	in->Load();
	if (in->GetParam().mIsEnable) {
		in->StartLoad();
	}
}

// 一時的なコマンドを必要に応じて提供する
void OneNoteCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	// 機能を利用しない場合は抜ける
	auto param = in->GetParam();
	if (param.mIsEnable == false) {
		return;
	}

	// プレフィックスが一致しない場合は抜ける
	const auto& prefix = param.mPrefix;
	bool hasPrefix = prefix.IsEmpty() == FALSE;
	if (hasPrefix && prefix.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	// 問い合わせ文字列の長さが閾値を下回る場合は機能を発動しない
	if (pattern->GetWholeTextLength() < param.mMinTriggerLength) {
		return;
	}

	auto repos = CommandRepository::GetInstance();

	int matchCount = 0;
	uint32_t ignoreMask = hasPrefix ? 1 : 0;

	CString pathStr;

	std::lock_guard<std::mutex> lock(in->mMutex); 
	if (in->mIsReady == false) {
		return;
	}
	for (auto& book : in->mBooks) {

		for (auto& section : book.mSections) {

			for (auto& page : section.GetPages()) {

				if (matchCount >= SEARCH_LIMIT) {
					return;
				}

				if (repos->HasQueryRequest()) {
					// 後続のリクエストが来ている場合は検索を打ち切る
					return;
				}

				pathStr = section.GetName();
				pathStr += _T("/");
				pathStr += page.GetName();

				// セッション名と入力ワードがマッチするかを判定
				int level = pattern->Match(pathStr, ignoreMask);
				if (level == Pattern::Mismatch) {
					continue;
				}
				if (hasPrefix && level == Pattern::PartialMatch) {
					level = Pattern::FrontMatch;
				}

				commands.Add(CommandQueryItem(level, new OneNoteCommand(&param, book, section, page)));
				matchCount++;
			}
		}
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t OneNoteCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(OneNoteCommand::TypeDisplayName());
	return 1;
}

}}} // end of namespace launcherapp::commands::onenote

