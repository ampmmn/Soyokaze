#include "pch.h"
#include "ExecuteHistory.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include <map>
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace common {

struct ExecuteHistory::PImpl
{
	const CString& GetFilePath() {
		if (mFilePath.IsEmpty() == FALSE) {
			return mFilePath;
		}

		Path path(Path::APPDIR, _T("history.txt"));
		mFilePath = path;

		return mFilePath;
	}

	CString mFilePath;

	std::map<CString, std::list<HISTORY_ITEM> > mItemMap;

	// 読み込み済か?
	bool mIsLoaded = false;
};

ExecuteHistory::ExecuteHistory() : in(std::make_unique<PImpl>())
{
}

ExecuteHistory::~ExecuteHistory()
{
}

ExecuteHistory* ExecuteHistory::GetInstance()
{
	static ExecuteHistory inst;
	return &inst;
}

/**
 * 履歴情報の追加
 * @param[in] type     種別
 * @param[in] word     コマンド実行時に入力された文字列
 * @param[in] fullPath コマンド実行時に実際に実行するフルパス
 */
void ExecuteHistory::Add(
	const CString& type,
	const CString& word,
	const CString& fullPath
)
{
	auto pref = AppPreference::Get();
	if (pref->IsUseInputHistory() == false) {
		// 履歴機能を使用しない場合
		return;
	}

	HISTORY_ITEM item;
	item.mWord = word;
	item.mFullPath = fullPath;

	auto& items = in->mItemMap[type];
	for (auto it = items.begin(); it != items.end(); ++it) {

		if (it->mFullPath != fullPath) {
			continue;
		}

		item = *it;
		items.erase(it);
		items.push_front(item);
		return;
	}

	items.push_front(item);
	int limit = pref->GetHistoryLimit();
	ASSERT(limit >= 0);
	if (items.size() > limit) {
		items.resize(limit);
	}
}

/**
 * 履歴情報の追加
 */
void ExecuteHistory::Add(
	const CString& type,
	const CString& word
)
{
	auto pref = AppPreference::Get();
	if (pref->IsUseInputHistory() == false) {
		// 履歴機能を使用しない場合
		return;
	}
	HISTORY_ITEM item;
	item.mWord = word;

	auto& items = in->mItemMap[type];
	for (auto it = items.begin(); it != items.end(); ++it) {
		if (it->mWord != word) {
			continue;
		}

		item = *it;
		items.erase(it);
		items.push_front(item);
		return;
	}

	items.push_front(item);

	int limit = pref->GetHistoryLimit();
	if (items.size() > limit) {
		items.resize(limit);
	}
}

/**
 * 履歴情報の取得
 */
void ExecuteHistory::GetItems(
	const CString& type,
	ItemList& items
) const
{
	auto pref = AppPreference::Get();
	if (pref->IsUseInputHistory() == false) {
		// 履歴機能を使用しない場合
		return;
	}

	auto itFind = in->mItemMap.find(type);
	if (itFind == in->mItemMap.end()) {
		return ;
	}

	const auto& itemList = itFind->second;

	items.reserve(itemList.size());
	for (auto& item : itemList) {
		items.push_back(item);
	}
}

int ExecuteHistory::EraseItems(const CString& type, const std::set<CString>& words)
{
	auto itFind = in->mItemMap.find(type);
	if (itFind == in->mItemMap.end()) {
		return 0;
	}

	int n = 0;

	auto& itemList = itFind->second;
	for (auto& word : words) {

		for (auto it = itemList.begin(); it != itemList.end(); ) {
			if (word != it->mWord) {
				it++;
				continue;
			}

			it = itemList.erase(it);
		}
	}
	return n;
}

void ExecuteHistory::ClearAllItems()
{
	in->mItemMap.clear();
	in->mIsLoaded = true;
}

void ExecuteHistory::Save()
{
	if (in->mIsLoaded == false) {
		return;
	}

	FILE* fpOut = nullptr;
	try {
		auto& path = in->GetFilePath();
		CString filePathTmp(path);
		filePathTmp += _T(".tmp");

		if (_tfopen_s(&fpOut, filePathTmp, _T("w,ccs=UTF-8")) != 0 || fpOut == nullptr) {
			return ;
		}

		CString line;
		CStdioFile file(fpOut);

		for (auto& elem : in->mItemMap) {

			auto& type = elem.first;
			auto& items = elem.second;

			line.Format(_T("[%s]\n"), (LPCTSTR)type);
			file.WriteString(line);
			int i = 1;
			for (auto& item : items) {
				line.Format(_T("Item%d=%s\t%s\n"), i, (LPCTSTR)item.mWord, (LPCTSTR)item.mFullPath);
				file.WriteString(line);
				i++;
			}
		}

		file.Close();
		fclose(fpOut);

		// 最後に一時ファイルを書き戻す
		if (CopyFile(filePathTmp, path, FALSE)) {
			// 一時ファイルを消す
			DeleteFile(filePathTmp);
		}
	}
	catch(CFileException* e) {
		e->Delete();
		fclose(fpOut);
	}
}

void ExecuteHistory::Load()
{
	in->mIsLoaded = true;

	auto& path = in->GetFilePath();

	FILE* fpIn = nullptr;
	if (_tfopen_s(&fpIn, path, _T("r,ccs=UTF-8")) != 0 || fpIn == nullptr) {
		return;
	}

	// ファイルを読む
	CStdioFile file(fpIn);

	CString strCurSectionName;

	std::map<CString, std::list<HISTORY_ITEM> > itemMap;
	std::list<HISTORY_ITEM> items;

	CString line;
	while(file.ReadString(line)) {

		line.Trim();

		if (line.IsEmpty()) {
			continue;
		}

		if (line[0] == _T('[')) {

			if (strCurSectionName.IsEmpty() == FALSE) {
				itemMap[strCurSectionName] = items;
			}

			strCurSectionName = line.Mid(1, line.GetLength()-2);
			items.clear();
			continue;
		}

		int n = line.Find(_T('='));
		if (n == -1) {
			continue;
		}

		CString strKey = line.Left(n);
		strKey.Trim();

		CString strValue = line.Mid(n+1);
		strValue.Trim();

		n = 0;
		CString word = strValue.Tokenize(_T("\t"), n);
		if (word.IsEmpty()) {
			continue;
		}

		CString fullPath = strValue.Tokenize(_T("\t"), n);

		HISTORY_ITEM newItem;
		newItem.mWord = word;
		newItem.mFullPath = fullPath;

		items.push_back(newItem);
	}

	if (strCurSectionName.IsEmpty() == FALSE) {
		itemMap[strCurSectionName] = items;
		items.clear();
	}

	file.Close();
	fclose(fpIn);

	in->mItemMap.swap(itemMap);
}


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

