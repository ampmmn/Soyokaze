#include "pch.h"
#include "ChromiumBrowseHistory.h"
#include "matcher/Pattern.h"
#include "utility/AppProfile.h"
#include "utility/CharConverter.h"
#include "utility/SQLite3Database.h"
#include <thread>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace bookmarks {

using CharConverter = launcherapp::utility::CharConverter;
using SQLite3Database = launcherapp::utility::SQLite3Database; 

struct ChromiumBrowseHistory::PImpl
{
	PImpl()
	{
	}
	virtual ~PImpl()
	{
	}

	void WatchHistoryDB();

	bool IsAbort() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}
	void Abort() {
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}

	// 監視スレッドの完了を待機する(最大3秒)
	void WaitExit() {
		DWORD start = GetTickCount();
		while (GetTickCount() - start < 3000) {
			if (IsAbort()) {
				break;
			}
			Sleep(50);
		}
		Sleep(250);
	}

	std::mutex mMutex;
	bool mIsAbort;
	bool mIsExited;
	CString mId;
	CString mProfileDir;
	bool mIsUseURL;
	bool mIsUseMigemo;

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

void ChromiumBrowseHistory::PImpl::WatchHistoryDB()
{
	// 元の履歴ファイルのパスを生成
	CString dbOrgPath = mProfileDir;
	PathAppend(dbOrgPath.GetBuffer(MAX_PATH_NTFS), _T("History"));
	dbOrgPath.ReleaseBuffer();

	// 退避先のパスを生成
	CString dbDstPath;
	auto p = dbDstPath.GetBuffer(MAX_PATH_NTFS);
	CAppProfile::GetDirPath(p, MAX_PATH_NTFS);
	PathAppend(p, _T("tmp"));
	if (PathIsDirectory(p) == FALSE) {
		if (CreateDirectory(p, nullptr) == FALSE) {
			return;
		}
	}

	// 退避先のファイル名にPC名を含める(UserDirをOneDrive共有フォルダで運用している環境向けの処理)
	TCHAR computerName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD bufLen = MAX_COMPUTERNAME_LENGTH + 1;
	GetComputerName(computerName, &bufLen);

	CString fileName;
	fileName.Format(_T("%s-history-%s"), computerName, (LPCTSTR)mId);
	PathAppend(p, fileName);

	dbDstPath.ReleaseBuffer();

	int count = 0;
	while(IsAbort() == false) {

		Sleep(50);
		if (count++ <= 20) {
			continue;
		}
		count = 0;

		// ファイルがなければコピー
		bool shouldCopy = false;
		if (PathFileExists(dbDstPath) == FALSE) {
			shouldCopy = true;
		}
		else {
			// 日付が新しければコピー
			FILETIME ftOrg;
			GetLastUpdateTime(dbOrgPath, ftOrg);
			FILETIME ftDst;
			GetLastUpdateTime(dbDstPath, ftDst);

			shouldCopy = (memcmp(&ftOrg, &ftDst, sizeof(FILETIME)) != 0);
		}

		if (mHistoryDB.get() == nullptr && IsAbort() == false && shouldCopy) {
			std::lock_guard<std::mutex> lock(mMutex);

			mHistoryDB.reset();
			if (CopyFile(dbOrgPath, dbDstPath, FALSE) == FALSE) {
				continue;
			}
			if (PathFileExists(dbDstPath) == FALSE) {
				continue;
			}
		}

		if (!mHistoryDB) {
			try {
				auto db = std::make_unique<SQLite3Database>(dbDstPath);
				db->Query(_T("create view hoge (url, title) as select distinct urls.url,urls.title from visits inner join urls on visits.url = urls.id where urls.title is not null and urls.title is not '' and urls.url not like '%google.com/search%' order by visits.visit_time desc ;"), nullptr, nullptr);
				db->Query(_T("create table hoge2(url, title); insert into hoge2(url,title) select * from hoge;"), nullptr, nullptr);
				db->Query(_T("PRAGMA synchronous = OFF;vacuum;reindex;"), nullptr, nullptr);

				std::lock_guard<std::mutex> lock(mMutex);
				mHistoryDB.reset(db.release());
			}
			catch(...) {
				break;
			}
		}
	}
}

ChromiumBrowseHistory::ChromiumBrowseHistory(
		const CString& id,
	 	const CString& profileDir,
		bool isUseURL,
	 	bool isUseMigemo
) : 
	in(new PImpl)
{
	in->mIsAbort = false;
	in->mId = id;
	in->mProfileDir = profileDir;
	in->mIsUseURL = isUseURL;
	in->mIsUseMigemo = isUseMigemo;

	std::thread th([&]() {
			in->WatchHistoryDB();
	});
	th.detach();
}

ChromiumBrowseHistory::~ChromiumBrowseHistory()
{
	Abort();

}

void ChromiumBrowseHistory::Abort()
{
	in->Abort();
	in->WaitExit();
}

void ChromiumBrowseHistory::Query(
		Pattern* pattern,
	 	std::vector<ITEM>& items,
	 	int limit,
		DWORD timeout
)
{
		std::lock_guard<std::mutex> lock(in->mMutex);
		if (!in->mHistoryDB) {
			return;
		}
		// patternから検索ワード一覧を得る
		std::vector<Pattern::WORD> words;
		pattern->GetWords(words);

		if(words.empty()) {
			return;
		}
		std::reverse(words.begin(), words.end());

		// 得た検索ワードからsqlite3のクエリ文字列を生成する
		static LPCTSTR basePart = _T("select url,title from hoge2 where ");
		CString queryStr(basePart);

		bool isFirst = true;
		CString token;
		for(auto& word : words) {
			if (word.mMethod == Pattern::RegExp) {
				token.Format(_T("%s (title regexp '%s') "), isFirst ? _T("") : _T("and"), (LPCTSTR)word.mWord);
			}
			else {
				if (in->mIsUseURL) {
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

				if (GetTickCount() - param->mStart > param->mTimeout) {
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
			DWORD mStart = GetTickCount();
			DWORD mTimeout = 150;
		};

		// mHistoryDBに対し、問い合わせを行う
		local_param param;
		param.mTimeout = timeout;
		in->mHistoryDB->Query(queryStr, local_param::Callback, (void*)&param);

		items.swap(param.mItems);
}

}
}
}

