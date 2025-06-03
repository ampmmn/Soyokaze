#pragma once

namespace launcherapp { namespace commands { namespace activate_window {

class ActivateIndicatorWindow
{
	ActivateIndicatorWindow();
	~ActivateIndicatorWindow();
public:
	static ActivateIndicatorWindow* GetInstance();

	void Cover(HWND hwnd);
	void Uncover();

protected:
	HWND Create();

	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
	HWND mHwnd{nullptr};
	HBITMAP mBuffer{nullptr};
	CRect mRectWindow{0,0,0,0};
};



}}} // end of namespace launcherapp::commands::activate_window


