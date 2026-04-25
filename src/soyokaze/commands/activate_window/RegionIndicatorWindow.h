#pragma once

namespace launcherapp { namespace commands { namespace activate_window {

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

	LRESULT OnPaint(HWND hwnd, WPARAM wp, LPARAM lp);
	LRESULT OnTimer(HWND hwnd, WPARAM wp, LPARAM lp);
private:
	// 関連するウインドウ
	HWND mHwnd{nullptr};
	// 枠描画用のバッファ
	HBITMAP mBuffer{nullptr};
	// 領域座標(スクリーン座標系)
	CRect mRectWindow{0,0,0,0};

	// 特定のウインドウと連動しないモード
	bool mIsIndependent{false};
};



}}} // end of namespace launcherapp::commands::activate_window


