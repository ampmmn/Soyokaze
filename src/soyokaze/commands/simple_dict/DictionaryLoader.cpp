#include "pch.h"
#include "DictionaryLoader.h"
#include "commands/simple_dict/SimpleDictCommand.h"
#include "commands/simple_dict/SimpleDictionary.h"
#include "commands/simple_dict/ExcelWrapper.h"
#include "utility/TimeoutChecker.h"
#include "utility/SQLite3Database.h"
#include "utility/Path.h"
#include "utility/CharConverter.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "commands/common/Message.h" // for PopupMessage
#include <thread>
#include <mutex>
#include <map>
#include <list>

using CharConverter = launcherapp::utility::CharConverter;
using SQLite3Database = launcherapp::utility::SQLite3Database;
using SQLite3Statement = launcherapp::utility::SQLite3Statement;

namespace launcherapp {
namespace commands {
namespace simple_dict {

constexpr int64_t DBVERSION = 1;

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
	// Excelからデータを読み取る
	bool LoadFromExcelApp(const SimpleDictParam& param, std::vector<CString>& keys, std::vector<CString>& values, std::vector<CString>& values2);
	// キャッシュファイルを読む
	bool LoadCacheFile(const SimpleDictParam& param, std::vector<CString>& keys, std::vector<CString>& values, std::vector<CString>& values2);
	// キャッシュへの保存を行う
	bool SaveCacheFile(const SimpleDictParam& param, const std::vector<CString>& keys, const std::vector<CString>& values, const std::vector<CString>& values2);

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

		bool isLoadOK = false;

		// 初期化
		keys.clear();
		values.clear();
		values2.clear();

		// キャッシュから読み込みを試みる
		isLoadOK = LoadCacheFile(param, keys, values, values2);

		// Excelアプリケーションからロードを試みる
		if (isLoadOK == false) {
			isLoadOK = LoadFromExcelApp(param, keys, values, values2); 
		}

		if (IsAbort()) {
			SPDLOG_DEBUG(_T("Aborted."));
			continue;
		}
		if (isLoadOK == false) {
			continue;
		}

		UpdateDictData(cmd, keys, values, values2);

		spdlog::debug(_T("[SimpleDict]Completed loading dict data. name:{}"), (LPCTSTR)param.mName); 
	}
	mIsExited = true;
	spdlog::info(_T("[SimpleDict] Exit Watch Thread"));
}

bool DictionaryLoader::PImpl::LoadFromExcelApp(
		const SimpleDictParam& param,
	 	std::vector<CString>& keys,
	 	std::vector<CString>& values,
	 	std::vector<CString>& values2
)
{
	try {
		if (PathIsURL(param.mFilePath) == FALSE && Path::FileExists(param.mFilePath) == false) {
			// ファイルパスがURLでなく、かつ、ファイルが存在しない場合は読み込みをしない
			// (URLの場合は判別できないので、実際に開いてみる)
			return false;
		}

		ExcelApplication app;

		// キーを読みこむ
		if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeFront, keys) != 0) {
			spdlog::warn(_T("[SimpleDict]Failed to get key text. name:{}"), (LPCTSTR)param.mName);
			return false;
		}
		if (IsAbort()) {
			return false;
		}

		// 値を読み込む
		if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeBack, values) != 0) {
			spdlog::warn(_T("[SimpleDict]Failed to get value text. name:{}"), (LPCTSTR)param.mName);
			return false;
		}
		if (param.mRangeValue2.IsEmpty() == FALSE) {
			// 値2を読み込む
			if (app.GetCellText(param.mFilePath, param.mSheetName, param.mRangeValue2, values2) != 0) {
				spdlog::warn(_T("[SimpleDict]Failed to get value2 text. name:{}"), (LPCTSTR)param.mName);
				return false;
			}
		}

		// キャッシュに登録する
		SaveCacheFile(param, keys, values, values2);

		return true;
	}
	catch(...) {
		SPDLOG_ERROR(_T("An unexpected exception occurred!"));
		return false;
	}
}


static bool GetLastUpdateTime(LPCTSTR path, FILETIME& ftime)
{
	HANDLE h = CreateFile(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	GetFileTime(h, nullptr, nullptr, &ftime);
	CloseHandle(h);
	return true;
}

static bool CheckSchemaVersion(SQLite3Database& db, int64_t version)
{
	struct local_callback_version {
		static int Callback(void* p, int argc, char** argv, char**colName) {
			auto param = (local_callback_version*)p;
			param->mVersion = std::stoull(argv[0]);
			return 1;  // 一つだけ取得できればOK
		}
		int64_t mVersion = 0;   // DBから取得したバージョン
	} callback;

	db.Query(_T("SELECT LASTMODIFIEDTIME FROM TBL_UPDATETIME WHERE ID = '__VERSION__';"), local_callback_version::Callback, &callback);

	bool isSameVersion = (version == callback.mVersion);
	if (isSameVersion == false) {
		SPDLOG_DEBUG(_T("Scheme version differ. expect:{0} actual:{1}"), version, callback.mVersion);
	}
	return isSameVersion;
}

bool DictionaryLoader::PImpl::LoadCacheFile(
	const SimpleDictParam& param,
 	std::vector<CString>& keys,
 	std::vector<CString>& values,
 	std::vector<CString>& values2
)
{

	// 1. ファイルパス文字列を生成する
	Path cachePath(Path::APPDIRPERMACHINE, _T("simpledict_cache.db"));

	// 2. sqliteで開く。なければ新規作成
	SQLite3Database db(cachePath);

	// 3. Paramの値から一意なハッシュ的なものを生成
	CString idStr = param.GetIdentifier();

	// 4. ハッシュに対応した更新日時情報を得る
	CString queryStr;

	if (db.TableExists(_T("TBL_UPDATETIME")) == false) {
		// 4.1 更新日時情報テーブルがなければ新規作成
		db.Query(_T("CREATE TABLE IF NOT EXISTS TBL_UPDATETIME(ID TEXT PRIMARY KEY NOT NULL, LASTMODIFIEDTIME INTEGER NOT NULL, COMMANDNAME TEXT);"), nullptr, nullptr);
		// スキーマバージョンを設定
		queryStr.Format(_T("INSERT INTO TBL_UPDATETIME (ID, LASTMODIFIEDTIME, COMMANDNAME) VALUES ('__VERSION__', %d, '');"), DBVERSION);
		db.Query(queryStr, nullptr, nullptr);
	}

	if (CheckSchemaVersion(db, DBVERSION) == false) {
		db.Close();
		DeleteFile(cachePath);
		return false;
	}

	struct local_callback_updatetime {
		static int Callback(void* p, int argc, char** argv, char**colName) {
			auto param = (local_callback_updatetime*)p;
			param->mUpdateTime = std::stoull(argv[0]);
			return 1;  // 一つだけ取得できればOK
		}
		uint64_t mUpdateTime;   // キャッシュ側に登録された更新日時
	} callback;

	queryStr.Format(_T("SELECT LASTMODIFIEDTIME FROM TBL_UPDATETIME WHERE ID = '%s';"), (LPCTSTR)idStr);
	db.Query(queryStr, local_callback_updatetime::Callback, &callback);

	// 5. ファイルから更新日時情報を得る
	FILETIME ft;
	if (GetLastUpdateTime(param.mFilePath, ft) == FALSE) {
		spdlog::warn(_T("Failed to get filetime {}"), (LPCTSTR)param.mFilePath);
		return false;
	}
	uint64_t lastWriteTime = ((uint64_t)ft.dwLowDateTime) | (((uint64_t)ft.dwHighDateTime) << 32);

	// 6. 4と5を比較し、差異がある場合はfalseを返す
	if (lastWriteTime != callback.mUpdateTime) {
		return false;
	}

	// 7. 3のハッシュに対応するデータを読む select * from items where id=..
	struct local_callback_entry {
		static int Callback(void* p, int argc, char** argv, char**colName) {
			auto param = (local_callback_entry*)p;

			param->conv.Convert(argv[0], param->mTmpStr);
			param->mKeys->push_back(param->mTmpStr);

			param->conv.Convert(argv[1], param->mTmpStr);
			param->mValues->push_back(param->mTmpStr);

			if (param->mParam->mRangeValue2.IsEmpty() == FALSE) {
				param->conv.Convert(argv[2], param->mTmpStr);
				param->mValues2->push_back(param->mTmpStr);
			}

			return 0;
		}
		CharConverter conv;
		CString mTmpStr;
		std::vector<CString>* mKeys;
		std::vector<CString>* mValues;
		std::vector<CString>* mValues2;
		const SimpleDictParam* mParam;
	} callback2;
	callback2.mKeys = &keys;
	callback2.mValues = &values;
	callback2.mValues2 = &values2;
	callback2.mParam = &param;

	queryStr.Format(_T("SELECT KEY,VALUE,VALUE2 FROM TBL_ENTRIES WHERE ID = '%s';"), (LPCTSTR)idStr);
	db.Query(queryStr, local_callback_entry::Callback, &callback2);
	
	return true;
}

// キャッシュへの保存を行う
bool DictionaryLoader::PImpl::SaveCacheFile(
	const SimpleDictParam& param,
	const std::vector<CString>& keys,
	const std::vector<CString>& values,
	const std::vector<CString>& values2
)
{
	// 1. ファイルパス文字列を生成する
	Path cachePath(Path::APPDIRPERMACHINE, _T("simpledict_cache.db"));

	// 2. sqliteで開く。なければ新規作成
	SQLite3Database db(cachePath);

	// 3. Paramの値から一意なハッシュ的なものを生成
	CString idStr = param.GetIdentifier();

	// 4. ファイルから更新日時情報を得る
	FILETIME ft;
	if (GetLastUpdateTime(param.mFilePath, ft) == FALSE) {
		spdlog::warn(_T("Failed to get filetime {}"), (LPCTSTR)param.mFilePath);
		return false;
	}
	uint64_t lastWriteTime = ((uint64_t)ft.dwLowDateTime) | (((uint64_t)ft.dwHighDateTime) << 32);

	// 5. テーブルがなければ作成する
	if (db.TableExists(_T("TBL_ENTRIES")) == false) {
		// 4.1 更新日時情報テーブルがなければ新規作成
		db.Query(_T("CREATE TABLE IF NOT EXISTS TBL_ENTRIES(KEY TEXT, VALUE TEXT, VALUE2 TEXT, ID TEXT);"), nullptr, nullptr);
	}

	// 6. データを更新する。ハッシュに紐付けられた既存情報を削除したあと、今回の情報を登録する

	// 以前のデータをいったん全削除
	CString queryStr;
	queryStr.Format(_T("DELETE FROM TBL_ENTRIES WHERE ID = '%s';"), (LPCTSTR)idStr);
	db.Query(queryStr, nullptr, nullptr);

	db.Query(_T("BEGIN TRANSACTION;"), nullptr, nullptr);

	const char *sql = "INSERT INTO TBL_ENTRIES (KEY, VALUE, VALUE2, ID) VALUES (?, ?, ?, ?);";

	SQLite3Statement stmt;
	int rc = db.Prepare(sql, &stmt);
	if (rc != 0) {
		spdlog::error("Failed to prepare statement");
		return false;
	}

	size_t n = keys.size();
	for (size_t i = 0; i < n; ++i) {

		stmt.BindText(1, keys[i]);
		
		stmt.BindText(2, i < values.size() ? values[i] : _T(""));
		stmt.BindText(3, i < values2.size() ? values2[i] : _T(""));
		stmt.BindText(4, idStr);

		rc = stmt.Step();
		if (rc != 101) {   // 101:SQLITE_DONE
			spdlog::error("SQL error");
		}
		stmt.Reset();
	}

	db.Query(_T("END TRANSACTION;"), nullptr, nullptr);
	stmt.Finalize();

	// 7. ハッシュと更新日時情報を更新する
	queryStr.Format(_T("INSERT OR REPLACE INTO TBL_UPDATETIME (ID, LASTMODIFIEDTIME, COMMANDNAME) VALUES ('%s', %I64u, '%s');"),
			            (LPCTSTR)idStr, lastWriteTime, (LPCTSTR)param.mName);
	db.Query(queryStr, nullptr, nullptr);

	return true;
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

