#include "pch.h"
#include "ExecuteHistory.h"
#include "utility/AppProfile.h"
#include "utility/IniFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace commands {
namespace pathfind {


ExecuteHistory::ExecuteHistory()
{
}

ExecuteHistory::~ExecuteHistory()
{
}

/**
 * 履歴情報の追加
 */
void ExecuteHistory::Add(
	const CString& word,
	const CString& fullPath
)
{
	HISTORY_ITEM item;
	item.mWord = word;
	item.mFullPath = fullPath;

	for (auto it = mItems.begin(); it != mItems.end(); ++it) {
		if (it->mFullPath != fullPath) {
			continue;
		}

		item = *it;
		mItems.erase(it);
		mItems.push_front(item);
		return;
	}

	mItems.push_front(item);
	if (mItems.size() >= 128) {
		mItems.pop_back();
	}
}

/**
 * 履歴情報の取得
 */
void ExecuteHistory::GetItems(
	std::vector<HISTORY_ITEM>& items
) const
{
	items.reserve(mItems.size());
	for (auto& item : mItems) {
		items.push_back(item);
	}
}


void ExecuteHistory::Save()
{
	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetDirPath(path, MAX_PATH_NTFS);
	PathAppend(path, _T("pathfind.his"));

	CIniFile file;
	file.Open(path);


	TCHAR key[128];
	int index = 1;

	file.Write(_T("PathFind"), _T("Count"), (int)mItems.size());
	for (auto& item : mItems) {

		// 存在しないパスは保存対象にしない
		if (PathFileExists(item.mFullPath) == FALSE) {
			continue;
		}

		_stprintf_s(key, _T("Word%d"), index);
		file.Write(_T("PathFind"), key, (LPCTSTR)item.mWord);
		_stprintf_s(key, _T("FullPath%d"), index);
		file.Write(_T("PathFind"), key, (LPCTSTR)item.mFullPath);

		index++;
	}

	file.Close();
}

void ExecuteHistory::Load()
{
	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetDirPath(path, MAX_PATH_NTFS);
	PathAppend(path, _T("pathfind.his"));

	CIniFile file;
	file.Open(path);

	std::list<HISTORY_ITEM> items;

	TCHAR key[128];

	int count = file.GetInt(_T("PathFind"), _T("Count"));
	for (int i = 0; i < count; ++i) {

		int index= i + 1;

		HISTORY_ITEM item;

		_stprintf_s(key, _T("Word%d"), index);
		item.mWord = file.GetString(_T("PathFind"), key);
		_stprintf_s(key, _T("FullPath%d"), index);
		item.mFullPath = file.GetString(_T("PathFind"), key);

		// 存在しないパスは履歴情報から外す
		if (PathFileExists(item.mFullPath) == FALSE) {
			continue;
		}


		items.push_back(item);
	}

	file.Close();

	mItems.swap(items);
}


} // end of namespace pathfind
} // end of namespace commands
} // end of namespace soyokaze

