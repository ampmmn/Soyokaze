#pragma once

namespace launcherapp { namespace commands { namespace uiautomation {

class UIElementIndicatorWindow
{
	UIElementIndicatorWindow();
	~UIElementIndicatorWindow();
public:
	static UIElementIndicatorWindow* GetInstance();

	void Indicate(HWND hwnd, const CRect& rc);
	void ClearIndication();

protected:
	HWND Create();

	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
	HWND mHwnd{nullptr};
	HBITMAP mBuffer{nullptr};
	CRect mRectWindow{0,0,0,0};
};



}}} // end of namespace launcherapp::commands::uiautomation


