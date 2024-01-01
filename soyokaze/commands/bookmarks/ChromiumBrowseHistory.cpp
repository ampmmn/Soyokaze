#include "pch.h"
#include "ChromiumBrowseHistory.h"
#include "Pattern.h"
#include "utility/AppProfile.h"
#include "utility/CharConverter.h"
#include "utility/SQLite3Database.h"
#include <thread>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace bookmarks {

using CharConverter = soyokaze::utility::CharConverter;
using SQLite3Database = soyokaze::utility::SQLite3Database; 

struct ChromiumBrowseHistory::PImpl
{
	void WatchHistoryDB();

	bool IsAbort() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}
	void Abort() {
		std::lock_guard<std::mutex> lock(mMutex);
		mIsAbort = true;
	}

	std::mutex mMutex;
	bool mIsAbort;
	CString mId;
	CString mProfileDir;

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
	CString fileName;
	fileName.Format(_T("history-%s"), mId);
	PathAppend(p, fileName);
	dbDstPath.ReleaseBuffer();

	while(IsAbort() == false) {

		Sleep(250);

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

		if (IsAbort() == false && shouldCopy) {
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
				mHistoryDB = std::make_unique<SQLite3Database>(dbDstPath);
			}
			catch(...) {
				break;
			}
		}
	}
}

ChromiumBrowseHistory::ChromiumBrowseHistory(
		const CString& id,
	 	const CString& profileDir
) : 
	in(new PImpl)
{
	in->mIsAbort = false;
	in->mId = id;
	in->mProfileDir = profileDir;

	std::thread th([&]() {
			in->WatchHistoryDB();
	});
	th.detach();
}

ChromiumBrowseHistory::~ChromiumBrowseHistory()
{
}

void ChromiumBrowseHistory::Abort()
{
	in->Abort();
}

void ChromiumBrowseHistory::Query(
		Pattern* pattern,
	 	std::vector<ITEM>& items,
	 	int limit
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
		static LPCTSTR basePart = _T("select distinct urls.url,urls.title from visits left join urls on visits.url = urls.id where urls.title is not null ");

		CString queryStr(basePart);

		CString token;
		for(auto& word : words) {
			if (word.mMethod == Pattern::RegExp) {
				token.Format(_T("and (urls.title regexp '%s') "), word.mWord);
			}
			else {
				token.Format(_T("and (urls.title like '%%%s%%' or urls.url like '%%%s%%') "), word.mWord, word.mWord);
			}
			queryStr += token;
		}

		// 履歴が新しい順に20件を上限に検索
		CString footer;
		footer.Format(_T(" order by visits.visit_time desc limit %d ;"), limit);
		queryStr += footer;

		struct local_param {
			static int Callback(void* p, int argc, char** argv, char**colName) {
				auto param = (local_param*)p;

				ITEM item;
				param->conv.Convert(argv[0], item.mUrl);
				param->conv.Convert(argv[1], item.mTitle);
				param->mItems.push_back(item);
				return 0;
			}

			std::vector<ITEM> mItems;
			CharConverter conv;
		};

		// mHistoryDBに対し、問い合わせを行う
		local_param param;
		int n = in->mHistoryDB->Query(queryStr, local_param::Callback, (void*)&param);

		items.swap(param.mItems);
}

}
}
}

