#include "pch.h"
#include "OperationWatcher.h"
#include "LauncherWindowEventListenerIF.h"
#include "setting/AppPreferenceListenerIF.h"
#include "LauncherWindowEventDispatcher.h"
#include "setting/AppPreference.h"
#include "mainwindow/WarnWorkTimeToast.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Toast = launcherapp::mainwindow::Toast;

struct OperationWatcher::PImpl : public LauncherWindowEventListenerIF, public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		LauncherWindowEventDispatcher::Get()->AddListener(this);
	}
	virtual ~PImpl()
	{
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnLockScreenOccurred() override
	{
		mIsLocking = true;
	}

	void OnUnlockScreenOccurred() override
	{
		mIsLocking = false;
		mStartWorkTime = GetTickCount64();
		mIsWarned = false;

		// 通知を消す
		Toast toast;
		toast.Clear();

	}
	void OnTimer() override
	{
		if (mIsEnable == false) {
			// 機能が無効化されている
			return ;
		}
		if (mIsLocking) {
			// ロックしてるので不要
			return ;
		}
		if (mIsWarned) {
			// 警告済
			return;
		}

		// 前回ロック時からの経過時間を確認する
		uint64_t n = GetTickCount64() - mStartWorkTime;
		if (n <= (DWORD)(mTimeToWarn * 60 * 1000)) {
			return;
		}

		// 長時間連続稼働を警告する
		Toast toast;
		toast.SetThreshold(mTimeToWarn);
		toast.Show();

		// 一回警告したら休憩入れるまでは警告しない
		mIsWarned = true;
	}


	void OnAppFirstBoot() override
	{
	}
	void OnAppNormalBoot() override {}

	void OnAppPreferenceUpdated() override
	{
		AppPreference* pref = AppPreference::Get();
		mIsEnable = pref->IsWarnLongOperation();
		mTimeToWarn = pref->GetTimeToWarnLongOperation();
	}

	void OnAppExit() override
	{
	}

	bool mIsEnable = false;
	int mTimeToWarn = 0;

	HWND mWhd = nullptr;
	uint64_t mStartWorkTime = 0;
	bool mIsLocking = false;
	bool mIsWarned = false;

};


OperationWatcher::OperationWatcher() : in(new PImpl)
{
}

OperationWatcher::~OperationWatcher()
{
}

void OperationWatcher::StartWatch(CWnd* wnd)
{
	in->mWhd = wnd->GetSafeHwnd();
	in->OnAppPreferenceUpdated();

	in->mStartWorkTime = GetTickCount64();
	in->mIsLocking = false;
}

