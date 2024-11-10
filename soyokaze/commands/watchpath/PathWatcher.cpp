#include "pch.h"
#include "PathWatcher.h"
#include "utility/SHA1.h"
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

	bool IsAbort() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mIsAbort;
	}
	bool WatchPath();

	void NotifyPath(const CString& cmdName, const CString& message, const CString& detail);


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

	// 排他制御用
	std::mutex mMutex;

	// 監視対象(mapのキーはコマンド名)
	std::map<CString, WatchTarget*> mTargets;

	// 監視終了フラグ
	bool mIsAbort = false;
	// 監視スレッド終了済を表すフラグ
	bool mIsExited = false;
	// 
	bool mIsScreenLocked = false;
	// 監視スレッド
	std::unique_ptr<std::thread> mTask;
};

// 監視を開始
void PathWatcher::PImpl::StartWatch()
{
	std::lock_guard<std::mutex> lock(mMutex);

	// タスクが作成済なら何もしない
	if (mTask.get()) {
		return;
	}

	// 初回にタスクを生成する
	mTask.reset(new std::thread([&]() {

		uint64_t count = 0;
		while(IsAbort() == false) {

			// 5秒に1回(50msec * 100回)=5sec
			if (count++ >= 100 && IsLockWorkstation() == false) {
				WatchPath();
				count = 0;
			}
			Sleep(50);
		}

		// 終了処理
		std::lock_guard<std::mutex> lock(mMutex);
		for (auto& item : mTargets) {
			auto& target = item.second;
			delete target;
		}
		mIsExited = true;

	}));
	mTask->detach();
}

bool PathWatcher::PImpl::IsLockWorkstation()
{
	std::lock_guard<std::mutex> lock(mMutex);
	return mIsScreenLocked;
}

// ローカルパス向けの更新検知処理
bool PathWatcher::PImpl::WatchPath()
{
	std::lock_guard<std::mutex> lock(mMutex);
	for (auto& item : mTargets) {
		auto target = item.second;
		if (target->IsUpdated() == false) {
			continue;
		}

		auto cmdName = target->GetCommandName();
		auto title = target->GetTitle();
		auto detail = target->GetDetail();
		NotifyPath(cmdName, title, detail);
	}
	return true;
}

void PathWatcher::PImpl::NotifyPath(const CString& cmdName, const CString& message, const CString& detail)
{
	LPCTSTR msg = message.IsEmpty() == FALSE? (LPCTSTR)message : _T("更新を検知");

	if (ShortcutSettingPage::IsStartMenuExists()) {
		// スタートメニューにショートカットが登録されている場合はトーストを使う
		Toast toast;
		toast.SetCommandName(cmdName);
		toast.SetPath(detail);
		toast.SetMessage(msg);
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
	in->mIsAbort = false;
	in->mIsExited = false;

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
			delete it->second;
			in->mTargets.erase(it);
		}

		if (PathIsUNC(item.mPath) == FALSE) {
			in->mTargets[cmdName] = new LocalPathTarget(cmdName, item.mMessage, item.mPath, item.mInterval);
		}
		else {
			in->mTargets[cmdName] = new UNCPathTarget(cmdName, item.mMessage,item.mPath, item.mInterval);
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
	delete it->second;
	in->mTargets.erase(it);
}

}
}
}

