#pragma once

class LauncherWindowEventListenerIF
{
public:
	virtual ~LauncherWindowEventListenerIF() {}

	virtual void OnLockScreenOccurred() = 0;
	virtual void OnUnlockScreenOccurred() = 0;
	virtual void OnTimer() = 0;
	virtual void OnLancuherActivate() = 0;
	virtual void OnLancuherUnactivate() = 0;

};


