#include "pch.h"
#include "ExecuteHistory.h"
#include "utility/Path.h"
#include "setting/AppPreference.h"
#include <nlohmann/json.hpp>
#include <map>
#include <list>
#include <algorithm>
#include <fstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace launcherapp {
namespace commands {
namespace common {

using ItemMap = std::map<CString, std::list<HISTORY_ITEM> >;
using nlohmann::json;

struct ExecuteHistory::PImpl
{
	const CString& GetFilePath() {
		if (mFilePath.IsEmpty() == FALSE) {
			return mFilePath;
		}

		Path path(Path::APPDIRPERMACHINE, _T("history.json"));
		mFilePath = path;

		return mFilePath;
	}

	CString mFilePath;

	ItemMap mItemMap;

	std::map<CString, uint32_t> mExcludeWordMap;

	// 読み込み済か?
	bool mIsLoaded{false};
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

	// 除外リストに含まれる場合は追加しない
	for (const auto& item : in->mExcludeWordMap) {
		const auto& excludeWord = item.first;

		if (word.Find(excludeWord) != -1) {
			// 除外リストに該当した
			return;
		}
	}

	HISTORY_ITEM item{word};

	auto& items = in->mItemMap[type];
	auto it = std::find_if(items.begin(), items.end(),
		 	[word](auto item) { return item.mWholeWord == word; });
	if (it != items.end()) {
		// 同じものがあったら先頭に移動
		auto item_to_move = *it;
		items.erase(it);
		items.push_front(item_to_move);
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
			if (word != it->mWholeWord) {
				it++;
				continue;
			}

			it = itemList.erase(it);
		}
	}
	return n;
}

// 指定ワードを含む場合に履歴に追加しないようにする
uint32_t ExecuteHistory::AddExcludeWord(const CString& excludeWord)
{
	auto it = in->mExcludeWordMap.find(excludeWord);
	if (it == in->mExcludeWordMap.end()) {
		it = in->mExcludeWordMap.insert(std::make_pair(excludeWord, uint32_t(0))).first;
	}
	return ++it->second;
}

uint32_t ExecuteHistory::RemoveExcludeWord(const CString& excludeWord)
{
	auto it = in->mExcludeWordMap.find(excludeWord);
	if (it == in->mExcludeWordMap.end()) {
		return 0;
	}
	uint32_t n = --it->second;
	if (n == 0) {
		in->mExcludeWordMap.erase(it);
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

	// JSONオブジェクトを生成
	try {
		std::string tmp;

		json root;
		for (auto it = in->mItemMap.begin(); it != in->mItemMap.end(); ++it) {

			const auto& type = it->first;
			auto& histories = it->second;

			auto json_histories = json::array();
			for (auto& item : histories)  {
				json_histories.push_back(UTF2UTF(item.mWholeWord, tmp));
			}

			root[UTF2UTF(type, tmp)] = json_histories;
		}

		// 一時ファイルにJSONを出力
		CString filePathTmp(in->GetFilePath());
		filePathTmp += _T(".tmp");
		std::ofstream ofs(filePathTmp);
		ofs << root.dump(4);
		ofs.close();

		// 最後に一時ファイルを書き戻す
		if (CopyFile(filePathTmp, in->GetFilePath(), FALSE)) {
			// 一時ファイルを消す
			DeleteFile(filePathTmp);
		}
	}
	catch(const json::exception& e) {
		CString what;
		UTF2UTF(e.what(), what);
		spdlog::warn(_T("Failed to save history {0}: {1}"), (LPCTSTR)in->GetFilePath(), (LPCTSTR)what);
	}
}

void ExecuteHistory::Load()
{
	if (Path::FileExists(in->GetFilePath()) == false) {
		// 初回起動時はファイルが存在しないので読み込み処理をしない
		in->mIsLoaded = true;
		return;
	}

	try {
		CString path;

		ItemMap tmpMap;

		std::ifstream f(in->GetFilePath());
		json history_dict = json::parse(f);
		for (auto it = history_dict.begin(); it != history_dict.end(); ++it) {
			std::string key = it.key();

			std::list<HISTORY_ITEM> items;

			auto histories = it.value();
			for (auto it2 = histories.begin(); it2 != histories.end(); ++it2) {

				UTF2UTF(it2->get<std::string>(), path);

				// 除外リストに含まれる場合は追加しない
				bool shouldExclude = false;
				for (const auto& item : in->mExcludeWordMap) {
					const auto& excludeWord = item.first;

					if (path.Find(excludeWord) == -1) {
						continue;
					}
					// 除外リストに該当した
					shouldExclude = true;
					break;
				}

				if (shouldExclude) {
					continue;
				}
				items.push_back(HISTORY_ITEM{path});
			}
			CString tmpKey;
			tmpMap[UTF2UTF(key, tmpKey)] = items;
		}
		in->mItemMap.swap(tmpMap);
		in->mIsLoaded = true;
	}
	catch(const json::exception& e) {
		CString what;
		UTF2UTF(e.what(), what);
		spdlog::error(_T("Failed to load history {0}: {1}"), (LPCTSTR)in->GetFilePath(), (LPCTSTR)what);
		return ;
	}

}


} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp

