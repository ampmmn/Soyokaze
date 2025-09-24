#pragma once

namespace launcherapp { namespace commands { namespace activate_window {

class MenuEventListener
{
public:
	virtual ~MenuEventListener() {}
	virtual void OnRequestPutName(HWND hwnd) = 0;
	virtual void OnRequestClose(HWND hwnd) = 0;
};


}}}
