#pragma once

namespace launcherapp { namespace commands { namespace place_window_in_region {

class RegionIndicatorWindow
{
	RegionIndicatorWindow();
	~RegionIndicatorWindow();
public:
	static RegionIndicatorWindow* GetInstance();

	void SetIndependentMode(bool isIndependent);

	void Cover(RECT rc);
	void Uncover();

protected:
	HWND Create();
	void Draw();

	static LRESULT CALLBACK OnWindowProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);
private:
	HWND mHwnd{nullptr};
	HBITMAP mBuffer{nullptr};
	CRect mRectWindow{0,0,0,0};
	bool mIsIndependent{false};
};



}}} // end of namespace launcherapp::commands::place_window_in_region


