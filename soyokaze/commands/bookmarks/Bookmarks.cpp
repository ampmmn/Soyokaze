#include "pch.h"
#include "Bookmarks.h"
#include "utility/CharConverter.h" 
#include <fstream>
#include <vector>
#include <nlohmann/json.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using json = nlohmann::json;
using CharConverter = soyokaze::utility::CharConverter;


namespace soyokaze {
namespace commands {
namespace bookmarks {

// Chrome $USERPROFILE/AppData/Local/Google/Chrome/User Data/Default/Bookmarks
// Edge $USERPROFILE/AppData/Local/Microsoft/Edge/User Data/Default/Bookmarks

struct Bookmarks::PImpl
{
	TCHAR mChromeBookmarkPath[MAX_PATH_NTFS];
	FILETIME mChromeUpdateTime;
	std::vector<ITEM> mChromeItems;

	TCHAR mEdgeBookmarkPath[MAX_PATH_NTFS];
	FILETIME mEdgeUpdateTime;
	std::vector<ITEM> mEdgeItems;
};

Bookmarks::Bookmarks() : in(new PImpl)
{
	size_t reqLen = 0;

	// Chrome
	_tgetenv_s(&reqLen, in->mChromeBookmarkPath, MAX_PATH_NTFS, _T("USERPROFILE"));
	PathAppend(in->mChromeBookmarkPath, _T("AppData/Local/Google/Chrome/User Data/Default/Bookmarks"));
	memset(&in->mChromeUpdateTime, 0, sizeof(FILETIME));

	// Edge
	_tgetenv_s(&reqLen, in->mEdgeBookmarkPath, MAX_PATH_NTFS, _T("USERPROFILE"));
	PathAppend(in->mEdgeBookmarkPath, _T("AppData/Local/Microsoft/Edge/User Data/Default/Bookmarks"));
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

static void parseJSONObject(json& j, std::vector<ITEM>& items)
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
		ITEM item;


		auto name = j["name"].get<std::string>();
		CharConverter conv;
		// UTF-8に変換
		conv.Convert(name.c_str(), item.mName);

		item.mUrl = CString(CStringA(j["url"].get<std::string>().c_str()));
		items.push_back(item);
	}
}


bool Bookmarks::LoadChromeBookmarks(std::vector<ITEM>& items)
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
	std::vector<ITEM> tmp;

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

bool Bookmarks::LoadEdgeBookmarks(std::vector<ITEM>& items)
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
	std::vector<ITEM> tmp;

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
} // end of namespace soyokaze

