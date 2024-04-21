#include "pch.h"
#include "FilterExecutor.h"
#include "matcher/PartialMatchPattern.h"
#include "commands/core/CommandParameter.h"
#include <thread>
#include <mutex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace filter {


constexpr int MINIMUM_QUOTA = 64;

struct FilterExecutor::PImpl
{
	//  開始の指示を出す
	void Start(const CString& keyword) {

		std::lock_guard<std::mutex> lock(mMutex);

		mResult.clear();
		mKeyword = keyword;
		mCurIndex = 0;
		for (auto& h : mCompleteEvts) {
			::ResetEvent(h);
		}

		mWaitEvt->SetEvent();
	}

	// 完了の指示を出す
	void Abort() {
		std::lock_guard<std::mutex> lock(mMutex);
		mWaitEvt->SetEvent();
		mIsAbort = true;
	}

	// 完了指示が出ているかを問い合わせる
	bool IsAbort() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}

	// スレッド側の終了を待つ
	void WaitExited() {
		for(;;) {
			Sleep(50);
			std::lock_guard<std::mutex> lock(mMutex);
			if (std::count(mAllExited.begin(), mAllExited.end(), TRUE) == mAllExited.size()) {
				break;
			}
		}
	}

	// index番目のスレッドが処理を終えたことを報告する
	void NotifyComplete(int index) {
		::SetEvent(mCompleteEvts[index]);
	}

	// 1つのスレッドが担当するデータを取得する
	bool GetNextCandidates(std::vector<CString>& candidates, int threads) {

		std::lock_guard<std::mutex> lock(mMutex);

		if (mCurIndex >= mAllCandidates.size()) {
			// もうない
			return false;
		}

		if (threads == 0) { threads = 1; }

		// 担当分の数を決める(均等にスレッド数で分ける)
		size_t quota = (size_t)ceil(mAllCandidates.size() / (double)threads);
		if (quota < MINIMUM_QUOTA) {
			quota = MINIMUM_QUOTA;
		}
		size_t remaining = mAllCandidates.size() - mCurIndex;

		// n:担当数
		size_t n = remaining >= quota ? quota : remaining;

		// スレッドが担当する分の要素一覧をわたす
		candidates.clear();
		auto itCur = mAllCandidates.begin() + mCurIndex;
		candidates.insert(candidates.end(), itCur, itCur + n);

		// スレッドに渡した分だけ、位置を進める
		mCurIndex += n;

		return true;
	}

	// 検索対象の候補一覧
	std::vector<CString> mAllCandidates;
	// 検索結果
	std::vector<CString> mResult;

	// スレッド側が絞り込み開始を検知する(待つ)ためのイベント
	std::unique_ptr<CEvent> mWaitEvt;

	// 各スレッドが完了したことを依頼側が待つためのイベントの配列
	std::vector<HANDLE> mCompleteEvts;

	// 次にスレッドに渡す候補の位置
	size_t mCurIndex;

	// 絞り込み文字列
	CString mKeyword;

	// 排他制御用
	std::mutex mMutex;

	// 完了フラグ
	bool mIsAbort;

	// スレッドが終了済かどうかを表す真偽値の配列
	std::vector<BOOL> mAllExited;
};

FilterExecutor::FilterExecutor() : in(new PImpl)
{
	in->mIsAbort = false;
	in->mWaitEvt.reset(new CEvent(FALSE, TRUE));

	// スレッドを作成する
	size_t threads = std::thread::hardware_concurrency() + 1;
	in->mAllExited.resize(threads, FALSE);

	for (size_t index = 0; index < threads; ++index) {

		in->mCompleteEvts.push_back(CreateEvent(nullptr, TRUE, FALSE, nullptr));

		std::thread th([&, index, threads]() {

			BOOL* isExitPtr = &(in->mAllExited.front()) + index;

			HANDLE evtHandle = *(in->mWaitEvt);

			PartialMatchPattern pattern;
			CString candidate;

			std::vector<CString> candidates;

			for(;;) {

			  // 開始を待つ
				if (WaitForSingleObject(evtHandle, 2000) == WAIT_TIMEOUT) {
					continue;
				}

				// 待機完了の指示があったら終わる
				if (in->IsAbort() ) {
					break;
				}

				// 自スレッドが担当する要素を取得する
				if (in->GetNextCandidates(candidates, (int)threads) == false) {
					// 自分の分がなければ戻る
					in->NotifyComplete((int)index);
					Sleep(0);
					continue;
				}

				// 検索条件
				launcherapp::core::CommandParameter commandParam(in->mKeyword);
				pattern.SetParam(commandParam);

				// 担当ぶんの要素を処理する
				std::vector<CString> matchedItems;
				for (auto& candidate : candidates) {

					if (pattern.Match(candidate) == Pattern::Mismatch) {
						continue;
					}

					// 該当するものを候補として登録
					matchedItems.push_back(candidate);
				}

				// 該当した候補の一覧をマージ
				std::lock_guard<std::mutex> lock(in->mMutex);
				in->mResult.insert(in->mResult.end(), matchedItems.begin(), matchedItems.end());

				// 完了
				in->NotifyComplete((int)index);
			}

			// 終了したことをフラグでおしらせ
			std::lock_guard<std::mutex> lock(in->mMutex);
			*isExitPtr = TRUE;
		});
		th.detach();
	}
}

FilterExecutor::~FilterExecutor()
{
	in->Abort();
	in->WaitExited();

	for (auto& h : in->mCompleteEvts) {
		CloseHandle(h);
	}
}

void FilterExecutor::ClearCandidates()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mAllCandidates.clear();
}

void FilterExecutor::AddCandidates(const CString& item)
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mAllCandidates.push_back(item);
}

void FilterExecutor::Execute(const CString& keyword, std::vector<CString>& result)
{
	if (keyword.IsEmpty()) {
		// 入力キーワードが空文字の場合は全てを候補に追加する
		auto& allCandidates = in->mAllCandidates;
		result.insert(result.end(), allCandidates.begin(), allCandidates.end());
		return;
	}

	// 入力キーワードによる絞り込みを開始する
	// (コンストラクタで作成したスレッド側で行う)
	in->Start(keyword);

	// スレッドの作業完了を待つ
	WaitForMultipleObjects((int)in->mCompleteEvts.size(), &in->mCompleteEvts.front(), TRUE, INFINITE);

	// スレッドが待ち状態になるよう、イベントの状態を戻す
	in->mWaitEvt->ResetEvent();

	// 絞り込み結果を呼び出し側に返す
	result.swap(in->mResult);
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


