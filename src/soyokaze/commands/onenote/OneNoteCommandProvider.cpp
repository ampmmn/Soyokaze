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
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
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
	public LauncherWindowEventListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		LauncherWindowEventDispatcher::Get()->AddListener(this);
	}
	virtual ~PImpl()
	{
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);
		AppPreference::Get()->UnregisterListener(this);
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {}

// LauncherWindowEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {

		if (mParam.mIsEnable == false) {
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

		// 前回のロードが実行中のばあいは完了を待つ
		if (mLoadThread.joinable()) {
			mLoadThread.join();
		}

		// 別スレッドでノートブックのロードを実行する
		spdlog::info("Reload Onenote Notebook.");
		std::thread th([&]() {
			HRESULT hr = CoInitialize(nullptr);
			if (SUCCEEDED(hr)) {
				LoadNotebooks();
			}
			CoUninitialize();
		});
		mLoadThread.swap(th);
	}

	void OnLauncherActivate() override
	{
		mIsBackground = false;
	}
	void OnLauncherUnactivate() override
	{
		mIsBackground = true;
	}


	void Load();
	void LoadNotebooks() {
		std::vector<OneNoteBook> books;
		OneNoteAppProxy app;
		if (app.GetHierarchy(books) == false) {
			return;
		}

		std::lock_guard<std::mutex> lock(mMutex); 
		std::swap(mBooks, books);
		mLastLoadTime = GetTickCount64();
	}

	CommandParam mParam;
	std::vector<OneNoteBook> mBooks;
	std::mutex mMutex;
	bool mIsBackground{false};
	uint64_t mLastLoadTime{0};
	// ブックマーク読み込みスレッド
	std::thread mLoadThread;
};

/**
	レジストリを参照し、保存されたセッション名の一覧を取得する
*/
void OneNoteCommandProvider::PImpl::Load()
{
	auto pref = AppPreference::Get();
	mParam.Load((Settings&)pref->GetSettings());
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(OneNoteCommandProvider)


OneNoteCommandProvider::OneNoteCommandProvider() : in(std::make_unique<PImpl>())
{
}

OneNoteCommandProvider::~OneNoteCommandProvider()
{
	if (in->mLoadThread.joinable()) {
		in->mLoadThread.join();
	}
}

CString OneNoteCommandProvider::GetName()
{
	return _T("OneNote");
}

// 一時的なコマンドの準備を行うための初期化
void OneNoteCommandProvider::PrepareAdhocCommands()
{
	// 設定情報を読む
	in->Load();
	//
	if (in->mParam.mIsEnable) {
		in->LoadNotebooks();
	}
}

// 一時的なコマンドを必要に応じて提供する
void OneNoteCommandProvider::QueryAdhocCommands(
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

	// 問い合わせ文字列の長さが閾値を下回る場合は機能を発動しない
	if (pattern->GetWholeTextLength() < in->mParam.mMinTriggerLength) {
		return;
	}

	auto repos = CommandRepository::GetInstance();

	int matchCount = 0;
	uint32_t ignoreMask = hasPrefix ? 1 : 0;

	CString pathStr;

	std::lock_guard<std::mutex> lock(in->mMutex); 
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

				commands.Add(CommandQueryItem(level, new OneNoteCommand(&in->mParam, book, section, page)));
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

