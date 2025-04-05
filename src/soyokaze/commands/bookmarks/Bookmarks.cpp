#include "pch.h"
#include "Bookmarks.h"
#include "utility/Path.h" 
#include <fstream>
#include <vector>

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

// Chrome $USERPROFILE/AppData/Local/Google/Chrome/User Data/Default/Bookmarks
// Edge $USERPROFILE/AppData/Local/Microsoft/Edge/User Data/Default/Bookmarks

struct Bookmarks::PImpl
{
	Path mChromeBookmarkPath;
	FILETIME mChromeUpdateTime = {};
	std::vector<Bookmark> mChromeItems;

	Path mEdgeBookmarkPath;
	FILETIME mEdgeUpdateTime = {};
	std::vector<Bookmark> mEdgeItems;
};

Bookmarks::Bookmarks() : in(std::make_unique<PImpl>())
{
	size_t reqLen = 0;

	// Chrome
	_tgetenv_s(&reqLen, in->mChromeBookmarkPath, in->mChromeBookmarkPath.size(), _T("USERPROFILE"));
	in->mChromeBookmarkPath.Append(_T("AppData/Local/Google/Chrome/User Data/Default/Bookmarks"));
	in->mChromeBookmarkPath.Shrink();

	memset(&in->mChromeUpdateTime, 0, sizeof(FILETIME));

	// Edge
	_tgetenv_s(&reqLen, in->mEdgeBookmarkPath, in->mEdgeBookmarkPath.size(), _T("USERPROFILE"));
	in->mEdgeBookmarkPath.Append(_T("AppData/Local/Microsoft/Edge/User Data/Default/Bookmarks"));
	in->mEdgeBookmarkPath.Shrink();
	memset(&in->mEdgeUpdateTime, 0, sizeof(FILETIME));

}

Bookmarks::~Bookmarks()
{
}

static bool GetLastUpdateTime(LPCTSTR path, FILETIME& ftime)
{
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	GetFileTime(h, nullptr, nullptr, &ftime);
	CloseHandle(h);
	return true;
}

static void parseJSONObject(json& j, std::vector<Bookmark>& items)
{
	if (j["type"] == "folder") {
		auto children_array = j["children"];
		if (children_array.is_array() == false) {
			return;
		}
		for (auto it = children_array.begin(); it != children_array.end(); ++it) {
			parseJSONObject(*it, items);
		}
	}
	else if (j["type"] == "url") {
		Bookmark item;


		auto name = j["name"].get<std::string>();
		// UTF-8に変換
		UTF2UTF(name, item.mName);

		item.mUrl = CString(CStringA(j["url"].get<std::string>().c_str()));
		items.push_back(item);
	}
}


bool Bookmarks::LoadChromeBookmarks(std::vector<Bookmark>& items)
{
	FILETIME lastUpdate;
	if (GetLastUpdateTime(in->mChromeBookmarkPath, lastUpdate) == false) {
		return false;
	}

	if (memcmp(&in->mChromeUpdateTime, &lastUpdate, sizeof(FILETIME)) == 0) {
		// 前回から更新はないため、前回の情報を返す
		items = in->mChromeItems;
		return true;
	}

	// 読み直す
	std::vector<Bookmark> tmp;

	try {
		std::ifstream f(in->mChromeBookmarkPath);
		json bookmarks = json::parse(f);
		auto roots = bookmarks["roots"];
		for (auto it = roots.begin(); it != roots.end(); ++it) {
			std::string key = it.key();
			parseJSONObject(it.value(), tmp);
		}
	}
	catch(std::exception&) {
		return false;
	}

	in->mChromeItems.swap(tmp);
	in->mChromeUpdateTime = lastUpdate;
	return true;
}

bool Bookmarks::LoadEdgeBookmarks(std::vector<Bookmark>& items)
{
	FILETIME lastUpdate;
	if (GetLastUpdateTime(in->mEdgeBookmarkPath, lastUpdate) == false) {
		return false;
	}

	if (memcmp(&in->mEdgeUpdateTime, &lastUpdate, sizeof(FILETIME)) == 0) {
		// 前回から更新はないため、前回の情報を返す
		items = in->mEdgeItems;
		return true;
	}

	// 読み直す
	std::vector<Bookmark> tmp;

	try {
		std::ifstream f(in->mEdgeBookmarkPath);
		json bookmarks = json::parse(f);
		auto roots = bookmarks["roots"];
		for (auto it = roots.begin(); it != roots.end(); ++it) {
			parseJSONObject(it.value(), tmp);
		}
	}
	catch(std::exception&) {
		return false;
	}

	in->mEdgeItems.swap(tmp);
	in->mEdgeUpdateTime = lastUpdate;
	return true;
}


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

