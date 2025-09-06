#include "pch.h"
#include "Bookmarks.h"
#include "utility/Path.h" 
#include "utility/LocalDirectoryWatcher.h" 
#include <fstream>
#include <vector>
#include <mutex>

#pragma warning( push )
#pragma warning( disable : 26800 26819 )
#include <nlohmann/json.hpp>
#pragma warning( pop )

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;

namespace launcherapp { namespace commands { namespace bookmarks {

static void parseJSONObject(json& j, const std::string& parentFolderPath, std::vector<Bookmark>& items)
{
	if (j["type"] == "folder") {
		auto children_array = j["children"];
		if (children_array.is_array() == false) {
			return;
		}

		std::string folderPath(parentFolderPath);
		if (folderPath.empty() == false) {
			folderPath += "/";
		}
		folderPath += j["name"].get<std::string>();

		for (auto it = children_array.begin(); it != children_array.end(); ++it) {
			parseJSONObject(*it, folderPath, items);
		}
	}
	else if (j["type"] == "url") {
		Bookmark item;

		auto name = j["name"].get<std::string>();

		UTF2UTF(name, item.mName);
		UTF2UTF(parentFolderPath, item.mFolderPath);
		UTF2UTF(j["url"].get<std::string>(), item.mUrl);

		items.push_back(item);
	}
}

struct Bookmarks::PImpl
{
	// ブックマークデータを読み込む
	bool LoadBookmarks();
	// ブックマーク要素がパターンと合致するかを判定
	int MatchBookmarkItem(Pattern* pattern, const Bookmark& item, bool isUseURL);

	std::mutex mMutex;

	// ブックマークファイルのパス
	CString mFilePath;
	// ファイルから読み込まれたブックマーク要素
	std::vector<Bookmark> mItems;
	// ブックマーク要素が読み込み済か?
	bool mIsLoaded{false};
	// ブックマークファイルの変更を検知するLocalDirectoryWatcherの登録ID
	uint32_t mId{0};
};

bool Bookmarks::PImpl::LoadBookmarks()
{
	// 読み直す
	std::vector<Bookmark> tmp;
	try {
		std::ifstream f(mFilePath);
		json bookmarks = json::parse(f);
		auto roots = bookmarks["roots"];
		for (auto it = roots.begin(); it != roots.end(); ++it) {
			std::string key = it.key();
			parseJSONObject(it.value(), "", tmp);
		}
	}
	catch(std::exception&) {
		spdlog::warn(_T("failed to parse bookmark path:{}"), (LPCTSTR)mFilePath);
		return false;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	spdlog::debug("chrome bookmark item count {0} -> {1}", mItems.size(), tmp.size());
	mItems.swap(tmp);
	return true;
}

int Bookmarks::PImpl::MatchBookmarkItem(Pattern* pattern, const Bookmark& item, bool isUseURL)
{
	if (item.mUrl.Find(_T("javascript:")) == 0) {
		// ブックマークレットは対象外
		return Pattern::Mismatch;
	}

	// まずは名前で比較
	int level = pattern->Match(item.mName, 1);
	if (level != Pattern::Mismatch) {
		return level;
	}

	// 次にフォルダパスで比較
	if (item.mFolderPath.IsEmpty() == FALSE) {
		level = pattern->Match(item.mFolderPath, 1);
		if (level != Pattern::Mismatch) {
			return level;
		}
	}

	if (isUseURL == false) {
		// URLを絞り込みに使わない場合はここではじく
		return Pattern::Mismatch;
	}
	return pattern->Match(item.mUrl, 1);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


Bookmarks::Bookmarks() : in(std::make_unique<PImpl>())
{
}

Bookmarks::~Bookmarks()
{
	// LocalDirectoryWatcherの登録を解除する
	if (in->mId != 0) {
		auto watcher = LocalDirectoryWatcher::GetInstance();
		watcher->Unregister(in->mId);
		in->mId = 0;
	}
}

bool Bookmarks::Initialize(LPCTSTR bookmarkPath)
{
	// LocalDirectoryWatcherに登録済の場合は解除する
	auto watcher = LocalDirectoryWatcher::GetInstance();
	if (in->mId != 0) {
		watcher->Unregister(in->mId);
		in->mId = 0;
	}

	// 初回の処理
	in->mFilePath = bookmarkPath;

	// LocalDirectoryWatcherに対し変更通知登録を行い
	in->mId = watcher->Register((LPCTSTR)in->mFilePath, [](void* p) {
			spdlog::info("bookmarks updated.");
			// 通知を受けて即ロードするとファイルアクセスに失敗するので少し間を置く
			Sleep(250);
			auto thisPtr = (PImpl*)p;
			thisPtr->LoadBookmarks();
			}, in.get());

	// ブックマークを直接ロードする。次回以降はファイル更新時に変更通知を通じて再ロードをおこなう
	in->LoadBookmarks();

	in->mIsLoaded = true;
	return true;
}

void Bookmarks::Query(Pattern* pattern, std::vector<Bookmark>& items, bool isUseURL)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	if (in->mIsLoaded == false) {
		// まだ準備できてない
		return ;
	}

	for (auto& item : in->mItems) {

		int level = in->MatchBookmarkItem(pattern, item, isUseURL);
		if (level == Pattern::Mismatch) {
			continue;
		}

		Bookmark newItem(item);
		newItem.mMatchLevel = level;

		items.push_back(newItem);
	}
}


}}} // end of namespace launcherapp::commands::bookmarks

