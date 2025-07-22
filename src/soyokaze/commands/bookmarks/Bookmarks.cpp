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


namespace launcherapp {
namespace commands {
namespace bookmarks {

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



// Chrome $USERPROFILE/AppData/Local/Google/Chrome/User Data/Default/Bookmarks
// Edge $USERPROFILE/AppData/Local/Microsoft/Edge/User Data/Default/Bookmarks

struct Bookmarks::PImpl
{
	bool LoadChromeBookmarks();
	bool LoadEdgeBookmarks();

	std::mutex mMutex;

	Path mChromeBookmarkPath{Path::USERPROFILE, _T("AppData\\Local\\Google\\Chrome\\User Data\\Default\\Bookmarks")};
	std::vector<Bookmark> mChromeItems;
	bool mIsChromeLoaded{false};

	Path mEdgeBookmarkPath{Path::USERPROFILE, _T("AppData\\Local\\Microsoft\\Edge\\User Data\\Default\\Bookmarks")};
	std::vector<Bookmark> mEdgeItems;
	bool mIsEdgeLoaded{false};
};

bool Bookmarks::PImpl::LoadChromeBookmarks()
{
	// 読み直す
	std::vector<Bookmark> tmp;
	try {
		std::ifstream f(mChromeBookmarkPath);
		json bookmarks = json::parse(f);
		auto roots = bookmarks["roots"];
		for (auto it = roots.begin(); it != roots.end(); ++it) {
			std::string key = it.key();
			parseJSONObject(it.value(), "", tmp);
		}
	}
	catch(std::exception&) {
		spdlog::warn(_T("failed to parse bookmark path:{}"), (LPCTSTR)mChromeBookmarkPath);
		return false;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	spdlog::debug("chrome bookmark item count {0} -> {1}", mChromeItems.size(), tmp.size());
	mChromeItems.swap(tmp);
	return true;
}

bool Bookmarks::PImpl::LoadEdgeBookmarks()
{
	// 読み直す
	std::vector<Bookmark> tmp;

	try {
		std::ifstream f(mEdgeBookmarkPath);
		json bookmarks = json::parse(f);
		auto roots = bookmarks["roots"];
		for (auto it = roots.begin(); it != roots.end(); ++it) {
			parseJSONObject(it.value(), "", tmp);
		}
	}
	catch(std::exception&) {
		spdlog::warn(_T("failed to parse bookmark path:{}"), (LPCTSTR)mEdgeBookmarkPath);
		return false;
	}

	std::lock_guard<std::mutex> lock(mMutex);
	spdlog::debug("edge bookmark item count {0} -> {1}", mEdgeItems.size(), tmp.size());
	mEdgeItems.swap(tmp);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



Bookmarks::Bookmarks() : in(std::make_unique<PImpl>())
{
}

Bookmarks::~Bookmarks()
{
}

bool Bookmarks::LoadChromeBookmarks(std::vector<Bookmark>& items)
{
	if (in->mIsChromeLoaded == false) {
		// 初回の処理

		// 変更通知登録を行い
		auto watcher = LocalDirectoryWatcher::GetInstance();
		watcher->Register((LPCTSTR)in->mChromeBookmarkPath, [](void* p) {
				spdlog::info("Chrome bookmarks updated.");
				// 通知を受けて即ロードするとファイルアクセスに失敗するので少し間を置く
				Sleep(250);
				auto thisPtr = (PImpl*)p;
				thisPtr->LoadChromeBookmarks();
		}, in.get());

		// ブックマークを直接ロードする。次回以降はファイル更新時に変更通知を通じて再ロードをおこなう
		in->LoadChromeBookmarks();

		in->mIsChromeLoaded = true;
	}

	std::lock_guard<std::mutex> lock(in->mMutex);
	items = in->mChromeItems;

	return true;
}

bool Bookmarks::LoadEdgeBookmarks(std::vector<Bookmark>& items)
{
	if (in->mIsEdgeLoaded == false) {
		// 初回の処理

		// 変更通知登録を行い
		auto watcher = LocalDirectoryWatcher::GetInstance();
		watcher->Register((LPCTSTR)in->mEdgeBookmarkPath, [](void* p) {
				spdlog::info("Edge bookmarks updated.");
				// 通知を受けて即ロードするとファイルアクセスに失敗するので少し間を置く
				Sleep(250);
				auto thisPtr = (PImpl*)p;
				thisPtr->LoadEdgeBookmarks();
		}, in.get());

		// ブックマークを直接ロードする。次回以降はファイル更新時に変更通知を通じて再ロードをおこなう
		in->LoadEdgeBookmarks();

		in->mIsEdgeLoaded = true;
	}

	std::lock_guard<std::mutex> lock(in->mMutex);
	items = in->mEdgeItems;

	return true;
}

} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

