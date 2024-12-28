#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace presentation {

class PowerPointWindow
{
public:
	PowerPointWindow();
	~PowerPointWindow();

	// ウインドウをアクティブにする
	bool Activate(bool isShowMaximize);
	// 指定したスライド番号に移動
	bool GoToSlide(int16_t pageIndex);

	static bool GetAcitveWindow(std::unique_ptr<PowerPointWindow>& ptr);
	static HWND FindPresentaionWindowHwnd(const CString& caption);

	static HICON ResolveIcon();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace presentation
} // end of namespace commands
} // end of namespace launcherapp

