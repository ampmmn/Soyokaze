#include "pch.h"
#include "ChromiumBrowseHistory.h"
#include "matcher/Pattern.h"
#include "utility/Path.h"
#include "utility/SQLite3Database.h"
#include "utility/LocalDirectoryWatcher.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace webhistory {

using SQLite3Database = launcherapp::utility::SQLite3Database; 

struct ChromiumBrowseHistory::PImpl
{
	PImpl()
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
		Path dbDstPath(Path::APPDIRPERMACHINE, _T("tmp"));
		if (dbDstPath.IsDirectory() == false) {
			if (CreateDirectory(dbDstPath, nullptr) == FALSE) {
				spdlog::warn(_T("Failed to create directory {}"), (LPCTSTR)dbDstPath);
				return;
			}
		}

		CString fileName;
		fileName.Format(_T("history-%s"), (LPCTSTR)mId);

		dbDstPath.Append(fileName);
		mDBFilePath = dbDstPath;
	}

	void UpdateDatabase();
	void Reload();

	bool Query(const std::vector<PatternInternal::WORD>& words, std::vector<ITEM>& items, int limit, DWORD timeout);

	CString mId;
	CString mProfileDir;
	CString mOrgDBFilePath;
	CString mDBFilePath;
	bool mIsUseURL{false};
	bool mIsUseMigemo{false};
	bool mIsFirstCall{true};
	std::mutex mMutex;

	std::unique_ptr<SQLite3Database> mHistoryDB;
};


void ChromiumBrowseHistory::PImpl::UpdateDatabase()
{
	if (mIsFirstCall == false) {
		return;
	}

	mIsFirstCall = false;

	Reload();

	LocalDirectoryWatcher::GetInstance()->Register(mOrgDBFilePath, [](void* p) {
			auto thisPtr = (PImpl*)p;
			thisPtr->Reload();
	}, this);
}

void ChromiumBrowseHistory::PImpl::Reload()
{
	std::lock_guard<std::mutex> lock(mMutex);
	spdlog::info(_T("Reload webhistory {}"), (LPCTSTR)mId);

	mHistoryDB.reset();

	if (CopyFile(mOrgDBFilePath, mDBFilePath, FALSE) == FALSE) {
		spdlog::warn(_T("Failed to copy history database."));
	}

	try {
		// データベースファイルをロード
		auto db = std::make_unique<SQLite3Database>(mDBFilePath);

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
		const std::vector<PatternInternal::WORD>& words,
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
		if (word.mMethod == PatternInternal::RegExp && mIsUseMigemo) {
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
			UTF2UTF(argv[0], item.mUrl);
			UTF2UTF(argv[1], item.mTitle);
			param->mItems.push_back(item);
			return 0;
		}

		std::vector<ITEM> mItems;
		uint64_t mStart = GetTickCount64();
		DWORD mTimeout = 150;
	};

	// mHistoryDBに対し、問い合わせを行う
	std::lock_guard<std::mutex> lock(mMutex);

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
		const std::vector<PatternInternal::WORD>& words,
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

