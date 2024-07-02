#include "pch.h"
#include "FilterExecutor.h"
#include "commands/filter/FilterCommandParam.h"
#include "matcher/PartialMatchPattern.h"
#include "commands/core/CommandParameter.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/Clipboard.h"
#include "utility/LocalPathResolver.h"
#include "utility/CharConverter.h"
#include "utility/LastErrorString.h"
#include "utility/Pipe.h"
#include "SharedHwnd.h"
#include <thread>
#include <mutex>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

using LocalPathResolver = launcherapp::utility::LocalPathResolver;
using CharConverter = launcherapp::utility::CharConverter;

namespace launcherapp {
namespace commands {
namespace filter {

struct CANDIDATE_ITEM
{
	CANDIDATE_ITEM(int index, const CString& name) :
	 	mIndex(index), mMatchLevel(Pattern::Mismatch), mDisplayName(name)
	{
	}
	CANDIDATE_ITEM(int index, int level, const CString& name) :
	 	mIndex(index), mMatchLevel(level), mDisplayName(name)
	{
	}

	CANDIDATE_ITEM(const CANDIDATE_ITEM&) = default;

	int mIndex;
	int mMatchLevel;
	CString mDisplayName;
};

using CandidateList = std::vector<CANDIDATE_ITEM>;

constexpr int MINIMUM_QUOTA = 64;

struct FilterExecutor::PImpl
{

	bool LoadCandidatesFromSubProcess(const CommandParam& param);
	bool LoadCandidatesFromDefinedValue(const CommandParam& param);
	bool LoadCandidatesFromClipboard(const CommandParam& param);
	void MakeCandidatesFromString(const CString& text);


	//  開始の指示を出す
	void StartQuery(const CString& keyword) {

		std::lock_guard<std::mutex> lock(mMutex);

		mResult.clear();
		mKeyword = keyword;
		mCurIndex = 0;
		for (auto& h : mCompleteEvents) {
			::ResetEvent(h);
		}
		for (auto& h : mWaitEvents) {
			::SetEvent(h);
		}
	}

	// 完了の指示を出す
	void Abort() {
		std::lock_guard<std::mutex> lock(mMutex);
		for (auto& h : mWaitEvents) {
			::SetEvent(h);
		}
		mIsAbort = true;
	}

	void SetLoaded()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsLoaded = true;
	}
	bool IsLoaded()
	{
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsLoaded;
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

	void ResetWaitEvents() {
		for (auto h : mWaitEvents) {
			ResetEvent(h);
		}
	}

	// index番目のスレッドが処理を終えたことを報告する
	void NotifyComplete(int index) {
		::SetEvent(mCompleteEvents[index]);
		::ResetEvent(mWaitEvents[index]);
	}

	// 1つのスレッドが担当するデータを取得する
	bool GetNextCandidates(CandidateList& candidates, int threads) {

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

		auto itEnd = (mAllCandidates.begin() + mCurIndex + n);
		int itemIndex = (int)mCurIndex;
		for (auto itCur = mAllCandidates.begin() + mCurIndex; itCur != itEnd; ++itCur) {
			candidates.push_back(CANDIDATE_ITEM(itemIndex++, *itCur));
		}

		// スレッドに渡した分だけ、位置を進める
		mCurIndex += n;

		return true;
	}

	// 検索対象の候補一覧
	std::vector<CString> mAllCandidates;
	// 検索結果
	CandidateList mResult;

	// スレッド側が絞り込み開始を検知する(待つ)ためのイベント
	std::vector<HANDLE> mWaitEvents;

	// 各スレッドが完了したことを依頼側が待つためのイベントの配列
	std::vector<HANDLE> mCompleteEvents;

	// 次にスレッドに渡す候補の位置
	size_t mCurIndex;

	LONG mRefCount = 1;

	// 絞り込み文字列
	CString mKeyword;

	// 排他制御用
	std::mutex mMutex;

	// 完了フラグ
	bool mIsAbort;

	// 候補生成済フラグ
	bool mIsLoaded = false;

	// スレッドが終了済かどうかを表す真偽値の配列
	std::vector<BOOL> mAllExited;
};

// 子プロセスの出力から候補一覧を生成する
bool FilterExecutor::PImpl::LoadCandidatesFromSubProcess(const CommandParam& param)
{
	// 子プロセスの出力を受け取るためのパイプを作成する
	Pipe pipeForStdout;
	Pipe pipeForStderr;

	PROCESS_INFORMATION pi = {};

	STARTUPINFO si = {};
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = pipeForStdout.GetWriteHandle();
	si.hStdError = pipeForStderr.GetWriteHandle();
	si.wShowWindow = SW_HIDE;

	CString path = param.mPath;

	ExpandMacros(path);

	LocalPathResolver resolver;
	resolver.Resolve(path);

	CString commandLine = _T(" ") + param.mParameter;
	ExpandMacros(commandLine);

	CString workDirStr;
	if (param.mDir.GetLength() > 0) {
		workDirStr = param.mDir;
		ExpandMacros(workDirStr);
		StripDoubleQuate(workDirStr);
	}

	LPCTSTR workDir = workDirStr.IsEmpty() ? nullptr : (LPCTSTR)workDirStr;
	BOOL isOK = CreateProcess(path, commandLine.GetBuffer(commandLine.GetLength()), NULL, NULL, TRUE, 0, NULL, workDir, &si, &pi);
	commandLine.ReleaseBuffer();

	if (isOK == FALSE) {
		LastErrorString errStr(GetLastError());
		spdlog::error(_T("Failed to run process ErrString:{}"), (LPCTSTR)errStr);
		return false;
	}

	CloseHandle(pi.hThread);

	HANDLE hRead = pipeForStdout.GetReadHandle();

	std::vector<char> output;
	std::vector<char> err;

	CharConverter conv;

	bool isExitChild = false;
	while(isExitChild == false) {

		if (isExitChild == false && WaitForSingleObject(pi.hProcess, 0) == WAIT_OBJECT_0) {
			isExitChild = true;

			DWORD exitCode;
			GetExitCodeProcess(pi.hProcess, &exitCode);

			if (exitCode != 0) {
				pipeForStderr.ReadAll(err);
				err.push_back(0x00);
				CString errStr;
				AfxMessageBox(conv.Convert(&err.front(), errStr));
				CloseHandle(pi.hProcess);
				return TRUE;
			}
		}

		pipeForStdout.ReadAll(output);
		pipeForStderr.ReadAll(err);
	}
	output.push_back(0x00);

	// ToDo: 文字コードを選べるようにする
	CString src;
	conv.Convert(&output.front(), src);

	CloseHandle(pi.hProcess);


	MakeCandidatesFromString(src);
	return true;
}

bool FilterExecutor::PImpl::LoadCandidatesFromDefinedValue(const CommandParam& param)
{
	// ToDo: 実装
	return true;
}

bool FilterExecutor::PImpl::LoadCandidatesFromClipboard(const CommandParam& param)
{
	CString src;
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 10, 0, (LPARAM)&src);

	if (src.IsEmpty()) {
		spdlog::info(_T("Clipboard is empty."));
		return false;
	}

	MakeCandidatesFromString(src);
	return true;
}

void FilterExecutor::PImpl::MakeCandidatesFromString(const CString& text)
{
	std::vector<CString> allCandidates;

	int n = 0;
	CString token = text.Tokenize(_T("\n"), n);
	while(token.IsEmpty() == FALSE) {
		token.Trim(_T(" \t\r"));
		allCandidates.push_back(token);
		token = text.Tokenize(_T("\n"), n);
	}

	std::lock_guard<std::mutex> lock(mMutex);
	mAllCandidates.swap(allCandidates);
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



FilterExecutor::FilterExecutor() : in(new PImpl)
{
	in->mIsAbort = false;

	// スレッドを作成する
	size_t threads = std::thread::hardware_concurrency();
	if (threads > 4) {
		threads = 4;
	}

	in->mAllExited.resize(threads, FALSE);

	for (size_t index = 0; index < threads; ++index) {

		constexpr BOOL isManualReset = TRUE;
		constexpr BOOL isInitialState = FALSE;

		in->mCompleteEvents.push_back(CreateEvent(nullptr, isManualReset, isInitialState, nullptr));
		in->mWaitEvents.push_back(CreateEvent(nullptr, isManualReset, isInitialState, nullptr));

		std::thread th([&, index, threads]() {

			BOOL* isExitPtr = &(in->mAllExited.front()) + index;

			HANDLE evtHandle = in->mWaitEvents[index];

			PartialMatchPattern pattern;
			CString candidate;

			CandidateList candidates;

			while (in->IsAbort() == false) {

			  // 開始を待つ
				if (WaitForSingleObject(evtHandle, 500) == WAIT_TIMEOUT) {
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
					continue;
				}

				// 検索条件
				launcherapp::core::CommandParameter commandParam(in->mKeyword);
				pattern.SetParam(commandParam);

				// 担当ぶんの要素を処理する
				CandidateList matchedItems;
				for (auto& candidate : candidates) {

					int level = pattern.Match(candidate.mDisplayName);
					if (level == Pattern::Mismatch) {
						continue;
					}

					// 該当するものを候補として登録
					matchedItems.push_back(CANDIDATE_ITEM(candidate.mIndex, level, candidate.mDisplayName));
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

	for (auto& h : in->mCompleteEvents) {
		CloseHandle(h);
	}
	for (auto& h : in->mWaitEvents) {
		CloseHandle(h);
	}
}

void FilterExecutor::LoadCandidates(const CommandParam& param)
{
	std::thread th([&, param]() {

		AddRef();

		if (param.mPreFilterType == 0) {
			// 子プロセスの出力から絞込み文字列を得る
			in->LoadCandidatesFromSubProcess(param);
		}
		else if (param.mPreFilterType == 1) {
			// クリップボードの値から
			in->LoadCandidatesFromClipboard(param);
		}
		else if (param.mPreFilterType == 2) {
			// 固定値
			in->LoadCandidatesFromDefinedValue(param);
		}

		in->SetLoaded();

		// 候補の抽出が完了したことを通知
		SharedHwnd sharedWnd;
		PostMessage(sharedWnd.GetHwnd(), WM_APP+15, 0, 0);

		Release();
	});
	th.detach();
}

bool FilterExecutor::IsLoaded()
{
	return in->IsLoaded();
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

size_t FilterExecutor::GetCandidatesCount()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	return in->mAllCandidates.size();
}

void FilterExecutor::Query(const CString& keyword, FilterResultList& result)
{
	if (keyword.IsEmpty()) {
		// 入力キーワードが空文字の場合は全てを候補に追加する
		std::lock_guard<std::mutex> lock(in->mMutex);
		for (auto& value: in->mAllCandidates) {
			result.push_back(FilterResult(Pattern::PartialMatch, value));
				// この時点では入力キーワードがないため、一致レベルは一律でPartialMatchとする
		}
		return;
	}

	// 入力キーワードによる絞り込みを開始する
	// (コンストラクタで作成したスレッド側で行う)
	in->StartQuery(keyword);

	// スレッドの作業完了を待つ
	WaitForMultipleObjects((int)in->mCompleteEvents.size(), &in->mCompleteEvents.front(), TRUE, INFINITE);

	// スレッドが待ち状態になるよう、イベントの状態を戻す
	in->ResetWaitEvents();

	// 絞り込み結果を呼び出し側に返す
	std::lock_guard<std::mutex> lock(in->mMutex);
	std::sort(in->mResult.begin(), in->mResult.end(), [](const CANDIDATE_ITEM& l, const CANDIDATE_ITEM& r) {
		return l.mIndex < r.mIndex;
	});

	result.clear();
	result.reserve(in->mResult.size());
	for (auto& item : in->mResult) {
		result.push_back(FilterResult(item.mMatchLevel, item.mDisplayName));
	}
	in->mResult.clear();
}

LONG FilterExecutor::AddRef()
{
	return InterlockedIncrement(&in->mRefCount);
}

LONG FilterExecutor::Release()
{
	auto n = InterlockedDecrement(&in->mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp


