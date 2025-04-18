#include "pch.h"
#include "ClipboardHistoryDB.h"

#include "commands/clipboardhistory/ClipboardHistoryCommand.h"
#include "matcher/PatternInternal.h"
#include "utility/SQLite3Database.h"
#include "utility/SHA1.h"
#include "utility/Path.h"
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

constexpr int64_t DBVERSION = 1; ///< データベースバージョン

using SQLite3Database = launcherapp::utility::SQLite3Database; 

/**
 * @brief ClipboardHistoryDB の内部実装クラス
 */
struct ClipboardHistoryDB::PImpl
{
	/**
	 * @brief クエリを実行して結果を取得する
	 * @param words 検索ワードのリスト
	 * @param result 結果を格納するリスト
	 */
	void Query(const std::vector<PatternInternal::WORD>& words, ResultList& result);

	/**
	 * @brief 全ての結果を取得する
	 * @param result 結果を格納するリスト
	 */
	void FetchAll(ResultList& result);

	int mNumOfResults{16}; ///< 結果の数
	int mSizeLimit{64}; ///< サイズ制限
	int mCountLimit{1024}; ///< カウント制限

	std::unique_ptr<SQLite3Database> mDB; ///< SQLite3 データベース
	std::mutex mMutex;
};

/**
 * @brief クエリを実行して結果を取得する
 * @param words 検索ワードのリスト
 * @param result 結果を格納するリスト
 */
void ClipboardHistoryDB::PImpl::Query(
	const std::vector<PatternInternal::WORD>& words,
	ResultList& result
)
{
	std::lock_guard<std::mutex> lock(mMutex);

	// 得た検索ワードからsqlite3のクエリ文字列を生成する
	static LPCTSTR basePart = _T("SELECT DISTINCT APPENDDATE,DATA from TBL_CLIPHISTORY where ");
	CString queryStr(basePart);

	bool isFirst = true;
	CString token;
	for(const auto& word : words) {
		if (word.mMethod == PatternInternal::RegExp) {
			token.Format(_T("%s (data regexp '%s') "), isFirst ? _T("") : _T("and"), (LPCTSTR)word.mWord);
		}
		else {
			token.Format(_T("%s data like '%%%s%%' "), isFirst ? _T(""): _T("and"), (LPCTSTR)word.mWord);
		}
		queryStr += token;
		isFirst = false;
	}

	CString footer(_T(" order by APPENDDATE DESC"));
	queryStr += footer;

	footer.Format(_T(" limit %d;"), mNumOfResults);
	queryStr += footer;

	struct local_param {
		static int Callback(void* p, int argc, char** argv, char**colName) {
			UNREFERENCED_PARAMETER(argc);
			UNREFERENCED_PARAMETER(colName);

			auto param = (local_param*)p;

			ITEM item;
			item.mAppendTime = std::stoull(argv[0]);
			UTF2UTF(argv[1], item.mData);

			param->mResultList->push_back(item);
			return 0;
		}

		ResultList* mResultList = nullptr;
	};

	// 問い合わせを行う
	local_param param;
	param.mResultList = &result;
	mDB->Query(queryStr, local_param::Callback, (void*)&param);
}

/**
 * @brief 全ての結果を取得する
 * @param result 結果を格納するリスト
 */
void ClipboardHistoryDB::PImpl::FetchAll(std::vector<ITEM>& result)
{
	std::vector<PatternInternal::WORD> words = { { _T("."),  PatternInternal::RegExp} };
	Query(words, result);
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/**
 * @brief コンストラクタ
 */
ClipboardHistoryDB::ClipboardHistoryDB() : in(new PImpl)
{
}

/**
 * @brief デストラクタ
 */
ClipboardHistoryDB::~ClipboardHistoryDB()
{
}

/**
 * @brief データベースをロードする
 * @param numResults 結果の数
 * @param sizeLimit サイズ制限
 * @param countLimit カウント制限
 * @return ロードに成功した場合は true、失敗した場合は false
 */
bool ClipboardHistoryDB::Load(int numResults, int sizeLimit, int countLimit)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	in->mNumOfResults = numResults;
	in->mSizeLimit = sizeLimit;
	in->mCountLimit = countLimit;

	// dbファイルを開く
	Path dbPath(Path::APPDIRPERMACHINE, _T("clipboardhistory.db"));
	in->mDB.reset(new SQLite3Database(dbPath));

	auto& db = *in->mDB.get();

	// FIXME: スキーマバージョンを調べて違っていたらファイル削除

	// 初回の場合はテーブルを作成する
	CString queryStr;

	if (in->mDB->TableExists(_T("TBL_CLIPHISTORY")) == false) {
		// 4.1 更新日時情報テーブルがなければ新規作成
		db.Query(_T("CREATE TABLE IF NOT EXISTS TBL_CLIPHISTORY(ID TEXT PRIMARY KEY NOT NULL, APPENDDATE INTEGER NOT NULL, DATA TEXT);"), nullptr, nullptr);
		// スキーマバージョンを設定
		queryStr.Format(_T("INSERT INTO TBL_CLIPHISTORY (ID, APPENDDATE, DATA) VALUES ('__VERSION__', %d, '');"), (int)DBVERSION);
		db.Query(queryStr, nullptr, nullptr);
	}

	// トリガー設定して上限を設ける
	db.Query(_T("DROP TRIGGER limit_clip_history_count;"), nullptr, nullptr);
	queryStr.Format(_T("CREATE TRIGGER limit_clip_history_count AFTER INSERT ON TBL_CLIPHISTORY WHEN (SELECT COUNT(*) FROM TBL_CLIPHISTORY) > %d BEGIN DELETE FROM TBL_CLIPHISTORY WHERE APPENDDATE = ( SELECT APPENDDATE FROM TBL_CLIPHISTORY ORDER BY APPENDDATE ASC LIMIT 1); END;"), countLimit);
	db.Query(queryStr, nullptr, nullptr);

	// FIXME: 件数を調べて、上限を超過しているものはここで超過分を削除

	return true;
}

/**
 * @brief データベースをアンロードする
 * @return アンロードに成功した場合は true、失敗した場合は false
 */
bool ClipboardHistoryDB::Unload()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	if (in->mDB.get()) {
		in->mDB->Close();
		in->mDB.reset();
	}
	return true;
}

/**
 * @brief クエリを実行して結果を取得する
 * @param pattern パターン
 * @param result 結果を格納するリスト
 */
void ClipboardHistoryDB::Query(Pattern* pattern, ResultList& result)
{
	RefPtr<PatternInternal> pat2;
	if (pattern->QueryInterface(IFID_PATTERNINTERNAL, (void**)&pat2) == false) {
		spdlog::error("failed to get IFID_PATTERNINTERNAL.");
		return ;
	}

	// patternから検索ワード一覧を得る
	std::vector<PatternInternal::WORD> words;
	pat2->GetWords(words);
	ASSERT(words.empty() == false);

	std::reverse(words.begin(), words.end());

	// prefixを除外
	words.pop_back();

	if (words.empty()) {
		// すべて取得
		in->FetchAll(result);
		return ;
	}

	in->Query(words, result);
}

/**
 * @brief クリップボードが更新された
 * @param data 更新されたデータ
 */
void ClipboardHistoryDB::UpdateClipboard(LPCTSTR data)
{
	// TCHAR(wchar_t)換算でサイズ上限を超える場合は無視する
	size_t len = _tcslen(data);
	if (len * sizeof(TCHAR) >= in->mSizeLimit * 1024) {
		spdlog::debug("copy data is too large.");
		return ;
	}

	// 重複チェック用のキーを生成
	SHA1 id;
	id.Add(data);

	// エスケープ
	CString escapedData(data);
	escapedData.Replace(_T("'"), _T("''"));
		
	// 現在時刻
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	uint64_t timeVal = ((uint64_t)ft.dwHighDateTime) << 32 | (uint64_t)ft.dwLowDateTime;

	CString idStr = id.Finish();

	CString queryStr;
	queryStr.Format(_T("INSERT INTO TBL_CLIPHISTORY (ID, APPENDDATE, DATA) VALUES ('%s', %lld, '%s');"),
	                (LPCTSTR)idStr, timeVal, (LPCTSTR)escapedData);

	// データベースに挿入
	std::lock_guard<std::mutex> lock(in->mMutex);
	int n = in->mDB->Query(queryStr, nullptr, nullptr);
	// 重複した場合はINSERTに失敗するのでAPPENDDATEを更新
	if (n != 0) {
		queryStr.Format(_T("UPDATE TBL_CLIPHISTORY SET APPENDDATE = %lld WHERE ID = '%s';"), timeVal, (LPCTSTR)idStr);
		in->mDB->Query(queryStr, nullptr, nullptr);
	}
}

} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp



