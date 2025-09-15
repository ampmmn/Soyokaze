#pragma once


namespace launcherapp { namespace actions { namespace activate_window {

class WindowTarget
{
public:
	virtual ~WindowTarget() {}

	virtual HWND GetHandle() = 0;

};

class SimpleWindowTarget : public WindowTarget
{
public:
	SimpleWindowTarget(HWND h) : mHwnd(h) {}

	HWND GetHandle() override { return mHwnd; }

	HWND mHwnd;
};



}}}

