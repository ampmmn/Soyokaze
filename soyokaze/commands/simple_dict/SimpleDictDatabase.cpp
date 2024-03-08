#include "pch.h"
#include "SimpleDictDatabase.h"
#include "commands/simple_dict/SimpleDictCommand.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "utility/TimeoutChecker.h"
#include <thread>
#include <mutex>
#include <map>
#include <list>

namespace soyokaze {
namespace commands {
namespace simple_dict {

struct RECORD
{
	RECORD(const CString& key, const CString& value) : mKey(key), mValue(value) {}
	RECORD(const RECORD&) = default;

	CString mKey;
	CString mValue;
};

struct DICTIONARY
{
	bool mIsMatchWithoutKeyword;
	bool mIsEnableReverse;
	std::vector<RECORD> mRecords;
};

struct SimpleDictDatabase::PImpl
{
	void StartWatch()
	{
		std::thread th([&]() {

				std::vector<CString> keys;
				std::vector<CString> values;
				while(IsAbort() == false) {

					Sleep(50);

					// 更新されたアイテムがあるまで待機
					SimpleDictParam param;
					if (NextUpdatedItem(param) == false) {
						continue;
					}
					// ToDo: ファイルの更新を監視できるようにする

					ExcelApplication app;

					keys.clear();
					values.clear();
					if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeFront, keys) != 0) {
						continue;
					}
					if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeBack, values) != 0) {
						continue;
					}
					UpdateDictData(param, keys, values);
				}
				mIsExited = true;
		});
		th.detach();
	}

	void Abort()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}

	// 監視スレッドの完了を待機する(最大3秒)
	void WaitExit() {
		DWORD start = GetTickCount();
		while (GetTickCount() - start < 3000) {
			if (mIsExited) {
				break;
			}
			Sleep(50);
		}
	}
	bool IsAbort()
	{
		return mIsAbort;
	}
	void AddUpdateQueue(const SimpleDictParam& param)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mUpdatedParam.push_back(param);
	}
	bool NextUpdatedItem(SimpleDictParam& param)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		if (mUpdatedParam.empty()) {
			return false;
		}
		auto it = mUpdatedParam.begin();
		param = *it;
		mUpdatedParam.erase(it);
		return true;
	}

	void UpdateDictData(const SimpleDictParam& param, const std::vector<CString>& keys, const std::vector<CString>& values)
	{
		size_t startIdx = param.mIsFirstRowHeader ? 1 : 0;

		std::lock_guard<std::mutex> lock(mMutex);
		auto& dictionary = mDictData[param.mName];

		// コマンド名がなくてもマッチするようにするか?
		dictionary.mIsMatchWithoutKeyword = param.mIsMatchWithoutKeyword;
		dictionary.mIsEnableReverse = param.mIsEnableReverse;

		size_t count = keys.size() < values.size() ? keys.size() : values.size();

		// データを更新
		auto& data = dictionary.mRecords;
		data.clear();
		for (size_t i = startIdx; i < count; ++i) {
			data.push_back(RECORD(keys[i], values[i]));
		}
	}
	void DeleteDictData(const SimpleDictParam& param)
	{
		std::lock_guard<std::mutex> lock(mMutex);
		auto it = mDictData.find(param.mName);
		if (it == mDictData.end()) {
			return;
		}
		mDictData.erase(it);
	}

	void MatchKeyValue(Pattern* pattern, std::vector<ITEM>& items, const CString& cmdName, const CString& keyStr,	const CString& valueStr,	bool isMatchWithoutKeyword);
	void MatchDict(Pattern* pattern, std::vector<ITEM>& items, int limit, utility::TimeoutChecker& tm, const CString& cmdName, const DICTIONARY& dictionary);

	// key:コマンド名, value: DICTIONARY
	std::map<CString, DICTIONARY> mDictData;  // ToDo: 自前でなくsqlite経由にしたい
	std::list<SimpleDictParam> mUpdatedParam;

	std::mutex mMutex;
	bool mIsAbort;
	bool mIsExited;
};


void SimpleDictDatabase::PImpl::MatchKeyValue(
	Pattern* pattern,
	std::vector<ITEM>& items,
	const CString& cmdName,
	const CString& keyStr,	
	const CString& valueStr,	
	bool isMatchWithoutKeyword
)
{
	if (isMatchWithoutKeyword) {
		//コマンド名の一致がなくても候補を表示する
		int level = pattern->Match(keyStr);
		if (level != Pattern::Mismatch) {
			return;
		}
		items.push_back(ITEM(level, cmdName, keyStr, valueStr));
		return ;
	}

	// コマンド名が一致しなければ候補を表示しない
	if (cmdName.CompareNoCase(pattern->GetFirstWord()) != 0) {
		return;
	}

	// コマンド名の後にキーワード指定がない場合はすべて列挙する
	if (pattern->GetWordCount() == 1) {
		items.push_back(ITEM(Pattern::WholeMatch, cmdName, keyStr, valueStr));
		return;
	}

	// コマンド名の後にキーワード指定がある場合はマッチングを行う。
	// ただし、先頭のコマンド名を除外するためoffset=1
	int level = pattern->Match(keyStr, 1);
	if (level == Pattern::Mismatch) {  
		return;
	}

	// 最低でも前方一致扱いにする(先頭のコマンド名は合致しているため)
	if (level == Pattern::PartialMatch) {
		level = Pattern::FrontMatch;
	}
	items.push_back(ITEM(level, cmdName, keyStr, valueStr));
}

void SimpleDictDatabase::PImpl::MatchDict(
	Pattern* pattern,
	std::vector<ITEM>& items,
	int limit,
	utility::TimeoutChecker& tm,
	const CString& cmdName,
	const DICTIONARY& dictionary
)
{
	// 辞書データをひとつずつ比較する
	for (const auto& record : dictionary.mRecords) {

		if (tm.IsTimeout()) {
			// 一定時間たっても終わらなかったらあきらめる
			return;
		}

		MatchKeyValue(pattern, items, cmdName, record.mKey, record.mValue, dictionary.mIsMatchWithoutKeyword);
		if (items.size() >= (size_t)limit) {
			return;
		}

		// 逆引きが有効ならキーと値を入れ替えてのマッチングも行う
		if (dictionary.mIsEnableReverse == false) {
			continue;
		}
		MatchKeyValue(pattern, items, cmdName, record.mValue, record.mKey, dictionary.mIsMatchWithoutKeyword);
		if (items.size() >= (size_t)limit) {
			return;
		}
	}
}




SimpleDictDatabase::SimpleDictDatabase() : in(new PImpl)
{
	in->mIsAbort = false;
	in->mIsExited = false;
	in->StartWatch();
}

SimpleDictDatabase::~SimpleDictDatabase()
{
	in->Abort();
	in->WaitExit();
}

void SimpleDictDatabase::Query(Pattern* pattern, std::vector<ITEM>& items, int limit, DWORD timeout)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	utility::TimeoutChecker tm(timeout);

	for (auto item : in->mDictData) {

		if (tm.IsTimeout()) {
			break;
		}

		// コマンド名
		const auto& cmdName = item.first;
		// 辞書情報
		const auto& dictionary = item.second;

		// 辞書データをひとつずつ比較する
		in->MatchDict(pattern, items, limit, tm, cmdName, dictionary);
	}
}

void SimpleDictDatabase::OnUpdateCommand(SimpleDictCommand* cmd)
{
	auto& param = cmd->GetParam();
	in->AddUpdateQueue(param);
}

void SimpleDictDatabase::OnDeleteCommand(SimpleDictCommand* cmd)
{
	auto& param = cmd->GetParam();
	in->DeleteDictData(param);
}

}
}
}

