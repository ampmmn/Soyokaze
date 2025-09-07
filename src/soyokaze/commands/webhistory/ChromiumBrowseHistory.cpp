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

// クエリのタイムアウト時間
constexpr uint64_t QUERY_TIMEOUT_MSEC = 150;

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
	}

	void UpdateDatabase();
	void Reload();

	bool Query(const std::vector<PatternInternal::WORD>& words, std::vector<ITEM>& items, int limit);

	bool MakeTempFilePath(CString& path)
	{
		CString fileName;
		fileName.Format(_T("history-%s-%I64x"), (LPCTSTR)mId, GetTickCount64());

		// 退避先のディレクトリを生成しておく
		Path dbDstPath(Path::APPDIRPERMACHINE, _T("tmp"));
		if (dbDstPath.IsDirectory() == false) {
			if (CreateDirectory(dbDstPath, nullptr) == FALSE) {
				spdlog::warn(_T("Failed to create directory {}"), (LPCTSTR)dbDstPath);
				return false;
			}
		}

		dbDstPath.Append(fileName);
		path = dbDstPath;
		return true;
	}

	CString mId;
	CString mProfileDir;
	CString mOrgDBFilePath;
	CString mDBFilePath;
	bool mIsUseURL{false};
	bool mIsUseMigemo{false};
	std::mutex mMutex;
	uint32_t mWatchId{0};

	std::unique_ptr<SQLite3Database> mHistoryDB;
};


void ChromiumBrowseHistory::PImpl::UpdateDatabase()
{
	// 以前の更新通知の登録があれば登録解除する
	if (mWatchId != 0) {
		LocalDirectoryWatcher::GetInstance()->Unregister(mWatchId);
		mWatchId = 0;
	}

	// オリジナルの履歴データベースファイルが更新されたら通知をもらうための登録をする
	mWatchId = LocalDirectoryWatcher::GetInstance()->Register(mOrgDBFilePath, [](void* p) {
			auto thisPtr = (PImpl*)p;
			// 更新があったときもリロード
			thisPtr->Reload();
	}, this);

	// リロード
	Reload();
}

void ChromiumBrowseHistory::PImpl::Reload()
{
	spdlog::info(_T("Reload webhistory {}"), (LPCTSTR)mId);

	CString dbFilePath;
	MakeTempFilePath(dbFilePath);
	if (CopyFile(mOrgDBFilePath, dbFilePath, FALSE) == FALSE) {
		spdlog::warn(_T("Failed to copy history database."));
	}

	try {
		// データベースファイルをロード
		auto db = std::make_unique<SQLite3Database>(dbFilePath);

		// 専用のviewを作成しておく
		db->Query(_T("create view hoge (url, title) as select distinct urls.url,urls.title from visits inner join urls on visits.url = urls.id where urls.title is not null and urls.title is not '' and urls.url not like '%google.com/search%' order by visits.visit_time desc ;"), nullptr, nullptr);
		db->Query(_T("create table hoge2(url, title); insert into hoge2(url,title) select * from hoge;"), nullptr, nullptr);
		db->Query(_T("PRAGMA synchronous = OFF;vacuum;reindex;"), nullptr, nullptr);

		// 新しくロードした方に置き換える
		std::lock_guard<std::mutex> lock(mMutex);
		mHistoryDB.reset(db.release());

		// 前のコピーを消し、新しいファイルバスを記憶しておく
		if (Path::FileExists(mDBFilePath)) {
			DeleteFile(mDBFilePath);
		}
		mDBFilePath = dbFilePath;
	}
	catch(...) {
		spdlog::warn(_T("An exception occurred!"));
	}
}

bool ChromiumBrowseHistory::PImpl::Query(
		const std::vector<PatternInternal::WORD>& words,
	 	std::vector<ITEM>& items,
	 	int limit
)
{
	if(words.empty()) {
		return true;
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

	spdlog::debug(_T("query str : {}"), (LPCTSTR)queryStr);

	struct local_param {
		static int Callback(void* p, int argc, char** argv, char**colName) {
			UNREFERENCED_PARAMETER(argc);
			UNREFERENCED_PARAMETER(colName);

			auto param = (local_param*)p;

			if (GetTickCount64()  > param->mTimeout) {
				return 1;
			}

			ITEM item;
			UTF2UTF(argv[0], item.mUrl);
			UTF2UTF(argv[1], item.mTitle);
			param->mItems.push_back(item);
			return 0;
		}
		uint64_t mTimeout{0};
	 	std::vector<ITEM> mItems;
	};

	// mHistoryDBに対し、問い合わせを行う
	std::lock_guard<std::mutex> lock(mMutex);
	if (mHistoryDB.get() == nullptr) {
		return false;
	}

	local_param param{ GetTickCount64() + QUERY_TIMEOUT_MSEC };
	mHistoryDB->Query(queryStr, local_param::Callback, (void*)&param);

	items.swap(param.mItems);

	return true;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ChromiumBrowseHistory::ChromiumBrowseHistory() : 
	in(new PImpl)
{
}

ChromiumBrowseHistory::~ChromiumBrowseHistory()
{
}

bool ChromiumBrowseHistory::Initialize(
		const CString& id,
	 	const CString& profileDir,
		bool isUseURL,
	 	bool isUseMigemo
)
{
	in->mId = id;
	in->mProfileDir = profileDir;
	in->mIsUseURL = isUseURL;
	in->mIsUseMigemo = isUseMigemo;

	in->InitFilePath();
	in->UpdateDatabase();

	return true;
}


bool ChromiumBrowseHistory::Query(
		const std::vector<PatternInternal::WORD>& words,
	 	std::vector<ITEM>& items,
	 	int limit
)
{
		return in->Query(words, items, limit);
}

}
}
}

