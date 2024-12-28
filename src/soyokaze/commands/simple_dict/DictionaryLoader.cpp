#include "pch.h"
#include "DictionaryLoader.h"
#include "commands/simple_dict/SimpleDictCommand.h"
#include "commands/simple_dict/SimpleDictionary.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "utility/TimeoutChecker.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/common/Message.h" // for PopupMessage
#include <thread>
#include <mutex>
#include <map>
#include <list>

namespace launcherapp {
namespace commands {
namespace simple_dict {

struct DictionaryLoader::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void StartWatch();

	void Abort()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}

	// 監視スレッドの完了を待機する(最大3秒)
	void WaitExit() {
		uint64_t start = GetTickCount64();
		while (GetTickCount64() - start < 3000) {
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
	void AddWaitingQueue(SimpleDictCommand* cmd)
	{
		// 別スレッド(StartWatch内のスレッド)で再読み込み処理を完了したらReleaseする
		cmd->AddRef();

		std::lock_guard<std::mutex> lock(mMutex);
		mUpdatedParamQueue.push_back(cmd);
	}

	bool NextUpdatedItem(SimpleDictCommand** cmd)
	{
		std::lock_guard<std::mutex> lock(mMutex);

		// キューに更新待ちのデータがあったらそれを処理する
		if (mUpdatedParamQueue.size() == 0) {
			return false;
		}

		auto it = mUpdatedParamQueue.begin();
		*cmd = *it;
		mUpdatedParamQueue.erase(it);

		SPDLOG_DEBUG(_T("Update dict(edit command). name:{0}"), (LPCTSTR)(*cmd)->GetName());
		return true;
	}

	// 辞書データの読み込みを行い、通知する
	void UpdateDictData(
			SimpleDictCommand* cmd,
		 	const std::vector<CString>& keys,
		 	const std::vector<CString>& values,
		 	const std::vector<CString>& values2
	)
	{
		SPDLOG_DEBUG(_T("args name:{}"), (LPCTSTR)cmd->GetName());

		auto param = cmd->GetParam();

		bool isSkipFirst = param.mIsFirstRowHeader != FALSE;

		std::lock_guard<std::mutex> lock(mMutex);
		Dictionary dictionary;

		size_t count = keys.size() < values.size() ? keys.size() : values.size();

		// データを更新
		for (size_t i = 0; i < count; ++i) {

			if (IsAbort()) {
				SPDLOG_DEBUG(_T("Aborted."));
				return;
			}

			// 空のデータは扱わない
			if (keys[i].IsEmpty() && values[i].IsEmpty()) {
				continue;
			}

			if (isSkipFirst) {
				// 「一行目をヘッダとして扱う」場合は初回のデータを無視する
				isSkipFirst = false;
				continue;
			}

			Record record(keys[i], values[i]);
			if (i < values2.size()) {
				record.mValue2 = values2[i];
			}
			dictionary.mRecords.push_back(record);
		}

		cmd->UpdateDictionary(dictionary);
	}

	void OnAppFirstBoot() override 
	{
		OnAppNormalBoot();
	}
	void OnAppNormalBoot() override 
	{
		StartWatch();
	}
	void OnAppPreferenceUpdated() override {}
	void OnAppExit() override 
	{
		Abort();
	}

	// 更新まちキュー
	std::list<SimpleDictCommand*> mUpdatedParamQueue;

	std::mutex mMutex;
	bool mIsAbort = false;
	bool mIsExited = true;
	uint64_t mLastUpdateCheckTime = 0;
};


void DictionaryLoader::PImpl::StartWatch()
{
	spdlog::info(_T("[SimpleDict] Start Watch Thread"));

	std::vector<CString> keys;
	std::vector<CString> values;
	std::vector<CString> values2;
	while(IsAbort() == false) {

		Sleep(50);

		// 更新されたアイテムがあるまで待機
		RefPtr<SimpleDictCommand> cmd;
		if (NextUpdatedItem(&cmd) == false) {
			ASSERT(cmd == nullptr);
			continue;
		}
		ASSERT(cmd.get());

		const auto& param = cmd->GetParam();

		spdlog::debug(_T("[SimpleDict]Start loading dict data. name:{}"), (LPCTSTR)param.mName); 

		try {
			ExcelApplication app;

			keys.clear();
			values.clear();
			values2.clear();
			if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeFront, keys) != 0) {
				spdlog::warn(_T("[SimpleDict]Failed to get key text. name:{}"), (LPCTSTR)param.mName);
				continue;
			}
			if (IsAbort()) {
				SPDLOG_DEBUG(_T("Aborted."));
				break;
			}
			if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeBack, values) != 0) {
				spdlog::warn(_T("[SimpleDict]Failed to get value text. name:{}"), (LPCTSTR)param.mName);
				continue;
			}
			if (param.mRangeValue2.IsEmpty() == FALSE) {
				if (IsAbort()) {
					SPDLOG_DEBUG(_T("Aborted."));
					break;
				}
				if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeValue2, values2) != 0) {
					spdlog::warn(_T("[SimpleDict]Failed to get value2 text. name:{}"), (LPCTSTR)param.mName);
					continue;
				}
			}
		}
		catch(...) {
			SPDLOG_ERROR(_T("An unexpected exception occurred!"));
			continue;
		}

		if (IsAbort()) {
			SPDLOG_DEBUG(_T("Aborted."));
			continue;
		}

		UpdateDictData(cmd, keys, values, values2);

		spdlog::debug(_T("[SimpleDict]Completed loading dict data. name:{}"), (LPCTSTR)param.mName); 
	}
	mIsExited = true;
	spdlog::info(_T("[SimpleDict] Exit Watch Thread"));
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



DictionaryLoader::DictionaryLoader() : in(new PImpl)
{
	std::thread th([&]() {
			in->StartWatch();
	});
	th.detach();
}

DictionaryLoader::~DictionaryLoader()
{
	in->Abort();
	in->WaitExit();
}

DictionaryLoader* DictionaryLoader::Get()
{
	static DictionaryLoader inst;
	return &inst;
}

void DictionaryLoader::AddWaitingQueue(SimpleDictCommand* cmd)
{
	in->AddWaitingQueue(cmd);
}


}
}
}

