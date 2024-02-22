#include "pch.h"
#include "SimpleDictDatabase.h"
#include "commands/simple_dict/SimpleDictCommand.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include <thread>
#include <mutex>
#include <map>
#include <list>

namespace soyokaze {
namespace commands {
namespace simple_dict {

struct SimpleDictDatabase::PImpl
{
	void StartWatch()
	{
		std::thread th([&]() {

				std::vector<CString> records;
				while(IsAbort() == false) {

					Sleep(50);

					// 更新されたアイテムがあるまで待機
					SimpleDictParam param;
					if (NextUpdatedItem(param) == false) {
						continue;
					}
					// ToDo: ファイルの更新を監視できるようにする

					ExcelApplication app;

					records.clear();
					if (app. GetCellText(param.mFilePath, param.mSheetName, param.mRange, records) != 0) {
						continue;
					}
					UpdateDictData(param, records);
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

	void UpdateDictData(const SimpleDictParam& param, const std::vector<CString> records)
	{
		size_t startIdx = param.mIsFirstRowHeader ? 1 : 0;

		std::lock_guard<std::mutex> lock(mMutex);
		auto& dictData = mDictData[param.mName];
		dictData.clear();
		dictData.insert(dictData.end(), records.begin() + startIdx, records.end());
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

	std::map<CString, std::vector<CString> > mDictData;  // ToDo: 自前でなくsqlite経由にしたい
	std::list<SimpleDictParam> mUpdatedParam;

	std::mutex mMutex;
	bool mIsAbort;
	bool mIsExited;
};


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

	DWORD start = GetTickCount();
	int hitCount = 0;

	for (auto item : in->mDictData) {
		const auto& records = item.second;
		for (const auto& record : records) {

			if (GetTickCount()-start > timeout) {
				return;
			}

			int level = pattern->Match(record);
			if (level == Pattern::Mismatch) {
				continue;
			}
			ITEM item;
			item.mRecord = record;
			items.push_back(item);
			hitCount++;
			if (hitCount >= limit) {
				return;
			}
		}
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

