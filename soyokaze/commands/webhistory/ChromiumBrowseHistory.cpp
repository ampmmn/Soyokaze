#include "pch.h"
#include "ChromiumBrowseHistory.h"
#include "matcher/Pattern.h"
#include "utility/AppProfile.h"
#include "utility/CharConverter.h"
#include "utility/SQLite3Database.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace webhistory {

using CharConverter = launcherapp::utility::CharConverter;
using SQLite3Database = launcherapp::utility::SQLite3Database; 

struct ChromiumBrowseHistory::PImpl
{
	PImpl() : mLastUpdatedTime({})
	{
	}
	virtual ~PImpl()
	{
	}

	void InitFilePath()
	{
		// 元の履歴ファイルのパスを生成
		mOrgDBFilePath = mProfileDir;
		PathAppend(mOrgDBFilePath.GetBuffer(MAX_PATH_NTFS), _T("History"));
		mOrgDBFilePath.ReleaseBuffer();

		// 退避先のパスを生成
		TCHAR dbDstPath[MAX_PATH_NTFS];
		CAppProfile::GetDirPath(dbDstPath, MAX_PATH_NTFS);
		PathAppend(dbDstPath, _T("tmp"));
		if (PathIsDirectory(dbDstPath) == FALSE) {
			if (CreateDirectory(dbDstPath, nullptr) == FALSE) {
				spdlog::warn(_T("Failed to create directory {}"), (LPCTSTR)dbDstPath);
				return;
			}
		}

		// 退避先のファイル名にPC名を含める(UserDirをOneDrive共有フォルダで運用している環境向けの処理)
		TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
		DWORD bufLen = MAX_COMPUTERNAME_LENGTH + 1;
		GetComputerName(computerName, &bufLen);

		CString fileName;
		fileName.Format(_T("%s-history-%s"), computerName, (LPCTSTR)mId);
		PathAppend(dbDstPath, fileName);
		mDBFilePath = dbDstPath;
	}

	void UpdateDatabase();

	bool Query(const std::vector<Pattern::WORD>& words, std::vector<ITEM>& items, int limit, DWORD timeout);

	CString mId;
	CString mProfileDir;
	CString mOrgDBFilePath;
	CString mDBFilePath;
	bool mIsUseURL = false;
	bool mIsUseMigemo = false;

	FILETIME mLastUpdatedTime = {};

	std::unique_ptr<SQLite3Database> mHistoryDB;
};


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

void ChromiumBrowseHistory::PImpl::UpdateDatabase()
{
	const auto& dbDstPath = mDBFilePath;

	// ファイルがなければコピー
	bool shouldCopy = false;
	if (PathFileExists(dbDstPath) == FALSE) {
		shouldCopy = true;
	}
	else {
		// ファイルがある場合は更新日時を比較し、異なっていたらコピー
		FILETIME ftSrc;
		GetLastUpdateTime(mOrgDBFilePath, ftSrc);
		shouldCopy = (memcmp(&ftSrc, &mLastUpdatedTime, sizeof(FILETIME)) != 0);
	}

	// 更新が必要と判断したら、既存のDBを破棄し、コピーしなおし
	if (shouldCopy == false && mHistoryDB.get()) {
		// リロード不要
		return;
	}

	mHistoryDB.reset();

	if (CopyFile(mOrgDBFilePath, dbDstPath, FALSE) == FALSE) {
		spdlog::warn(_T("Failed to copy history database."));
	}
	// コピー元ファイルの更新日時を覚えておく
	// (次回以降に更新があったかどうかの判断に用いる)
	GetLastUpdateTime(mOrgDBFilePath, mLastUpdatedTime);

	try {
		// データベースファイルをロード
		auto db = std::make_unique<SQLite3Database>(dbDstPath);

		// 専用のviewを作成しておく
		db->Query(_T("create view hoge (url, title) as select distinct urls.url,urls.title from visits inner join urls on visits.url = urls.id where urls.title is not null and urls.title is not '' and urls.url not like '%google.com/search%' order by visits.visit_time desc ;"), nullptr, nullptr);
		db->Query(_T("create table hoge2(url, title); insert into hoge2(url,title) select * from hoge;"), nullptr, nullptr);
		db->Query(_T("PRAGMA synchronous = OFF;vacuum;reindex;"), nullptr, nullptr);

		// 新しくロードした方に置き換える
		mHistoryDB.reset(db.release());
	}
	catch(...) {
		spdlog::warn(_T("An exception occurred!"));
	}
}

bool ChromiumBrowseHistory::PImpl::Query(
		const std::vector<Pattern::WORD>& words,
	 	std::vector<ITEM>& items,
	 	int limit,
	 	DWORD timeout
)
{
	if (mHistoryDB.get() == nullptr) {
		return false;
	}

	// 得た検索ワードからsqlite3のクエリ文字列を生成する
	static LPCTSTR basePart = _T("select distinct url,title from hoge2 where ");
	CString queryStr(basePart);

	bool isFirst = true;
	CString token;
	for(const auto& word : words) {
		if (word.mMethod == Pattern::RegExp) {
			if (mIsUseURL) {
				token.Format(_T("%s (title regexp '%s' or url regexp '%s') "), isFirst ? _T("") : _T("and"), (LPCTSTR)word.mWord, (LPCTSTR)word.mWord);
			}
			else {
				token.Format(_T("%s (title regexp '%s') "), isFirst ? _T("") : _T("and"), (LPCTSTR)word.mWord);
			}
		}
		else {
			if (mIsUseURL) {
				token.Format(_T("%s (title like '%%%s%%' or url like '%%%s%%') "), isFirst ? _T(""): _T("and"), (LPCTSTR)word.mWord, (LPCTSTR)word.mWord);
			}
			else {
				token.Format(_T("%s title like '%%%s%%' "), isFirst ? _T(""): _T("and"), (LPCTSTR)word.mWord);
			}
		}
		queryStr += token;
		isFirst = false;
	}

	// 検索件数の上限を設定
	CString footer;
	footer.Format(_T(" limit %d ;"), limit);
	queryStr += footer;

	struct local_param {
		static int Callback(void* p, int argc, char** argv, char**colName) {
			UNREFERENCED_PARAMETER(argc);
			UNREFERENCED_PARAMETER(colName);

			auto param = (local_param*)p;

			if (GetTickCount64() - param->mStart > param->mTimeout) {
				return 1;
			}

			ITEM item;
			param->conv.Convert(argv[0], item.mUrl);
			param->conv.Convert(argv[1], item.mTitle);
			param->mItems.push_back(item);
			return 0;
		}

		std::vector<ITEM> mItems;
		CharConverter conv;
		uint64_t mStart = GetTickCount64();
		DWORD mTimeout = 150;
	};

	// mHistoryDBに対し、問い合わせを行う
	local_param param;
	param.mTimeout = timeout;
	mHistoryDB->Query(queryStr, local_param::Callback, (void*)&param);

	items.swap(param.mItems);

	return true;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ChromiumBrowseHistory::ChromiumBrowseHistory(
		const CString& id,
	 	const CString& profileDir,
		bool isUseURL,
	 	bool isUseMigemo
) : 
	in(new PImpl)
{
	in->mId = id;
	in->mProfileDir = profileDir;
	in->mIsUseURL = isUseURL;
	in->mIsUseMigemo = isUseMigemo;

	in->InitFilePath();
}

ChromiumBrowseHistory::~ChromiumBrowseHistory()
{
}

bool ChromiumBrowseHistory::Query(
		const std::vector<Pattern::WORD>& words,
	 	std::vector<ITEM>& items,
	 	int limit,
	 	DWORD timeout
)
{
		if(words.empty()) {
			return true;
		}

		in->UpdateDatabase();

		return in->Query(words, items, limit, timeout);
}

}
}
}

