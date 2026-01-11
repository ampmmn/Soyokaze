#include "pch.h"
#include "PathWatcher.h"
#include "utility/SHA1.h"
#include <atomic>
#include <map>
#include <mutex>
#include <thread>
#include <vector>
#include <deque>
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "commands/common/Message.h"
#include "commands/watchpath/LocalPathTarget.h"
#include "commands/watchpath/UNCPathTarget.h"
#include "commands/watchpath/WatchPathToast.h"
#include "utility/ManualEvent.h"
#include "settingwindow/ShortcutSettingPage.h"

namespace launcherapp {
namespace commands {
namespace watchpath {

using ITEM = PathWatcher::ITEM;

struct PathWatcher::PImpl : public LauncherWindowEventListenerIF
{
	// 監視を開始
	void StartWatch();

	//
	bool IsLockWorkstation();

	// 監視を中止する
	void Abort() {
		mAbortEvent.Set();
	}
	// 監視スレッドの完了を待機する(最大3秒)
	void WaitExit() {
		mExitEvent.WaitFor(3000);
	}

	bool IsAbort() {
		return mAbortEvent.WaitFor(0);
	}
	bool ScanPath();

	void NotifyPath(const CString& cmdName, const CString& message, const CString& detail, const CString& path);


	void OnLockScreenOccurred() override
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsScreenLocked = true;
	}
	void OnUnlockScreenOccurred() override
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mIsScreenLocked = false;
	}
	void OnTimer() override
	{
	}
	void OnLauncherActivate() override
	{
	}
	void OnLauncherUnactivate() override
	{
	}

	// 排他制御用
	std::mutex mMutex;

	// 監視対象(mapのキーはコマンド名)
	std::map<CString, std::unique_ptr<WatchTarget> > mTargets;

	// 監視スレッド終了済を表すフラグ
	ManualEvent mExitEvent;
	// スクリーンロック中か?
	bool mIsScreenLocked{false};
	// 監視スレッド
	std::unique_ptr<std::thread> mTask;
	// 間隔
	uint32_t mScanInterval{5000};
	// 
	ManualEvent mAbortEvent;
};

// 監視を開始
void PathWatcher::PImpl::StartWatch()
{
	std::lock_guard<std::mutex> lock(mMutex);

	// タスクが作成済なら何もしない
	if (mTask.get()) {
		return;
	}

	// イベントの状態を初期化
	mAbortEvent.Reset();
	mExitEvent.Reset();

	// 初回にタスクを生成する
	mTask.reset(new std::thread([&]() {

		do {
			if (IsLockWorkstation()) {
				// スクリーンロック中に変更を検知して通知をしても通知を見れないため、ロック中はチェックしない。
				continue;
			}
			ScanPath();
		} while(mAbortEvent.WaitFor(mScanInterval) == false);

		// 終了処理
		std::lock_guard<std::mutex> lock(mMutex);
		mTargets.clear();
		mExitEvent.Set();
	}));
	mTask->detach();
}

bool PathWatcher::PImpl::IsLockWorkstation()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return mIsScreenLocked;
}

// ローカルパス向けの更新検知処理
bool PathWatcher::PImpl::ScanPath()
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto& item : mTargets) {
		auto& target = item.second;
		if (target->IsUpdated() == false) {
			continue;
		}

		auto cmdName = target->GetCommandName();
		auto title = target->GetTitle();
		auto detail = target->GetDetail();
		auto path = target->GetPath();
		NotifyPath(cmdName, title, detail, path);
	}
	return true;
}

void PathWatcher::PImpl::NotifyPath(
	const CString& cmdName,
 	const CString& message,
 	const CString& detail,
 	const CString& path
)
{
	LPCTSTR msg = message.IsEmpty() == FALSE? (LPCTSTR)message : _T("更新を検知");

	if (AppSettingPageShortcut::IsStartMenuExists()) {
		// スタートメニューにショートカットが登録されている場合はトーストを使う
		Toast toast;
		toast.SetCommandName(cmdName);
		toast.SetPath(path);
		toast.SetMessage(msg);
		toast.SetDetail(detail);
		toast.Show();
	}
	else {
		// 登録されていない場合はShell_NotifyIconのメッセージで代替する
		CString notifyMsg;
		notifyMsg.Format(_T("【%s】%s : %s"), (LPCTSTR)cmdName, msg, (LPCTSTR)detail);
		launcherapp::commands::common::PopupMessage(notifyMsg);
	}
}

PathWatcher::PathWatcher() : in(new PImpl)
{
	LauncherWindowEventDispatcher::Get()->AddListener(in.get());
}

PathWatcher::~PathWatcher()
{
	LauncherWindowEventDispatcher::Get()->RemoveListener(in.get());

	in->Abort();
	in->WaitExit();
}

PathWatcher* PathWatcher::Get()
{
	static PathWatcher inst;
	return &inst;
}

void PathWatcher::RegisterPath(const CString& cmdName, const ITEM& item)
{
	{
		std::lock_guard<std::mutex> lock(in->mMutex);

		auto it = in->mTargets.find(cmdName);
		if (it != in->mTargets.end()) {
			in->mTargets.erase(it);
		}

		if (PathIsUNC(item.mPath) == FALSE) {
			in->mTargets.insert(std::make_pair(cmdName,  new LocalPathTarget(cmdName, item)));
		}
		else {
			in->mTargets.insert(std::make_pair(cmdName, new UNCPathTarget(cmdName, item)));
		}
	}
	in->StartWatch();
}

void PathWatcher::UnregisterPath(const CString& cmdName)
{
	std::lock_guard<std::mutex> lock(in->mMutex);

	auto it = in->mTargets.find(cmdName);
	if (it == in->mTargets.end()) {
		return;
	}
	in->mTargets.erase(it);
}

}
}
}

