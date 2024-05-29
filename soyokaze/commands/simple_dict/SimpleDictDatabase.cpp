#include "pch.h"
#include "SimpleDictDatabase.h"
#include "commands/simple_dict/SimpleDictCommand.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "utility/TimeoutChecker.h"
#include "commands/common/Message.h" // for PopupMessage
#include <thread>
#include <mutex>
#include <map>
#include <list>

namespace launcherapp {
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

struct UPDATESTATE
{
	SimpleDictParam mParam;
	FILETIME mFtLastUpdated; 
};

static bool GetLastUpdateTime(LPCTSTR path, FILETIME& ftime)
{
	if (PathIsURL(path)) {
		// URLパスは非対応(ここでチェックしなくても後段のCreateFileで失敗するはずではあるが..)
		return false;
	}

	HANDLE h = CreateFile(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	GetFileTime(h, nullptr, nullptr, &ftime);
	CloseHandle(h);
	return true;
}


struct SimpleDictDatabase::PImpl
{
	void StartWatch()
	{
		std::thread th([&]() {

			spdlog::info(_T("[SimpleDict] Start Watch Thread"));

			std::vector<CString> keys;
			std::vector<CString> values;
			while(IsAbort() == false) {

				Sleep(50);

				// 更新されたアイテムがあるまで待機
				SimpleDictParam param;
				if (NextUpdatedItem(param) == false) {
					continue;
				}

				spdlog::debug(_T("[SimpleDict]Start loading dict data. name:{}"), (LPCTSTR)param.mName); 
				ExcelApplication app;

				keys.clear();
				values.clear();
				if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeFront, keys) != 0) {
					spdlog::warn(_T("[SimpleDict]Failed to get key text. name:{}"), (LPCTSTR)param.mName);
					continue;
				}
				if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeBack, values) != 0) {
					spdlog::warn(_T("[SimpleDict]Failed to get value text. name:{}"), (LPCTSTR)param.mName);
					continue;
				}
				UpdateDictData(param, keys, values);

				spdlog::debug(_T("[SimpleDict]Completed loading dict data. name:{}"), (LPCTSTR)param.mName); 
			}
			mIsExited = true;
			spdlog::info(_T("[SimpleDict] Exit Watch Thread"));
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
		mUpdatedParamQueue.push_back(param);

		// 更新チェック用に登録しておく
		UPDATESTATE updateState;
		updateState.mParam = param;
		if (GetLastUpdateTime(param.mFilePath, updateState.mFtLastUpdated)) {
			mParamMap[param.mName] = updateState;
		}
	}
	bool NextUpdatedItem(SimpleDictParam& param)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		// キューに更新待ちのデータがあったらそれを処理する
		if (mUpdatedParamQueue.size() > 0) {
			auto it = mUpdatedParamQueue.begin();
			param = *it;
			mUpdatedParamQueue.erase(it);

			SPDLOG_DEBUG(_T("Update dict(edit command). name:{0}"), (LPCTSTR)param.mName);
			return true;
		}

		// 更新されたデータソースがあったら再読み込みする
		auto elapsed = GetTickCount() - mLastUpdateCheckTime;
		if (elapsed <= 1000) {
			// ある程度間隔をあけてチェックする
			return false;
		}

		bool isUpdatedItemExist = false;
		for (auto& item : mParamMap) {
			auto& updateState = item.second;
			FILETIME ftUpdated;
			if (GetLastUpdateTime(updateState.mParam.mFilePath, ftUpdated) == false ||
					memcmp(&ftUpdated, &updateState.mFtLastUpdated, sizeof(FILETIME)) == 0) {
				continue;
			}

			param = updateState.mParam;
			updateState.mFtLastUpdated = ftUpdated;
			isUpdatedItemExist = true;

			if (param.mIsNotifyUpdate) {
				// 更新を通知する
				CString msg;
				msg.Format(_T("【%s】ファイルが更新されました\n%s"), param.mName, param.mFilePath);
				launcherapp::commands::common::PopupMessage(msg);
			}

			SPDLOG_DEBUG(_T("Update dict(timestamp). name:{0}"), (LPCTSTR)param.mName);
			break;
		}
		mLastUpdateCheckTime = GetTickCount();

		return isUpdatedItemExist;
	}

	// 辞書データの読み込み
	void UpdateDictData(
			const SimpleDictParam& param,
		 	const std::vector<CString>& keys,
		 	const std::vector<CString>& values
	)
	{
		SPDLOG_DEBUG(_T("args name:{}"), (LPCTSTR)param.mName);

		bool isSkipFirst = param.mIsFirstRowHeader != FALSE;

		std::lock_guard<std::mutex> lock(mMutex);
		auto& dictionary = mDictData[param.mName];

		// コマンド名がなくてもマッチするようにするか?
		dictionary.mIsMatchWithoutKeyword = param.mIsMatchWithoutKeyword;
		dictionary.mIsEnableReverse = param.mIsEnableReverse;

		size_t count = keys.size() < values.size() ? keys.size() : values.size();

		// データを更新
		auto& data = dictionary.mRecords;
		data.clear();
		for (size_t i = 0; i < count; ++i) {
			if (keys[i].IsEmpty() && values[i].IsEmpty()) {
				continue;
			}
			if (isSkipFirst) {
				// 「一行目をヘッダとして扱う」場合は初回のデータを無視する
				isSkipFirst = false;
				continue;
			}
			data.push_back(RECORD(keys[i], values[i]));
		}
	}

	void DeleteDictData(const CString& name)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		// コマンド名に対応する辞書データを削除する
		auto it = mDictData.find(name);
		if (it != mDictData.end()) {
			mDictData.erase(it);
		}

		// 更新チェック用のmapからも削除する
		auto it2 = mParamMap.find(name);
		if (it2 != mParamMap.end()) {
			mParamMap.erase(it2);
		}
	}

	void MatchKeyValue(Pattern* pattern, std::vector<ITEM>& items, const CString& cmdName, const CString& keyStr,	const CString& valueStr,bool isMatchWithoutKeyword, bool isEnableReverse);
	void MatchDict(Pattern* pattern, std::vector<ITEM>& items, int limit, utility::TimeoutChecker& tm, const CString& cmdName, const DICTIONARY& dictionary);

	// key:コマンド名, value: DICTIONARY
	std::map<CString, DICTIONARY> mDictData;  // ToDo: 自前でなくsqlite経由にしたい

	// 名前別のコマンドパラメータ(更新チェック用)
	std::map<CString, UPDATESTATE> mParamMap;

	// 更新まちキュー
	std::list<SimpleDictParam> mUpdatedParamQueue;

	std::mutex mMutex;
	bool mIsAbort = false;
	bool mIsExited = false;
	DWORD mLastUpdateCheckTime = 0;
};


void SimpleDictDatabase::PImpl::MatchKeyValue(
	Pattern* pattern,
	std::vector<ITEM>& items,
	const CString& cmdName,
	const CString& keyStr,
	const CString& valueStr,
	bool isMatchWithoutKeyword,
	bool isEnableReverse
)
{

	// コマンド名が一致しなければ候補を表示しない
	if (cmdName.CompareNoCase(pattern->GetFirstWord()) != 0) {

		if (isMatchWithoutKeyword == false) {
			return;
		}

		//コマンド名の一致がなくても候補を検索する
		int level = pattern->Match(keyStr);
		if (level != Pattern::Mismatch) {
			items.push_back(ITEM(level, cmdName, keyStr, valueStr));
			return;
		}
		if (isEnableReverse == false) {
			return;
		}
		// 逆引きが有効なら値でのマッチングも行う
		level = pattern->Match(valueStr);
		if (level != Pattern::Mismatch) {
			items.push_back(ITEM(level, cmdName, keyStr, valueStr));
			return;
		}
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
		if (isEnableReverse == false) {
			return;
		}

		// 逆引きが有効なら値でのマッチングも行う
		level = pattern->Match(valueStr, 1);
		if (level == Pattern::Mismatch) {  
			return;
		}
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

		MatchKeyValue(pattern, items, cmdName, record.mKey, record.mValue, dictionary.mIsMatchWithoutKeyword, dictionary.mIsEnableReverse);
		if (items.size() >= (size_t)limit) {
			return;
		}
	}
}




SimpleDictDatabase::SimpleDictDatabase() : in(new PImpl)
{
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

void SimpleDictDatabase::OnUpdateCommand(SimpleDictCommand* cmd, const CString& oldName)
{
	auto& param = cmd->GetParam();

	if (param.mName != oldName) {
		in->DeleteDictData(oldName);
	}
	in->AddUpdateQueue(param);
}

void SimpleDictDatabase::OnDeleteCommand(SimpleDictCommand* cmd)
{
	auto& param = cmd->GetParam();
	in->DeleteDictData(param.mName);
}

}
}
}

