#pragma once

#include <memory>

namespace launcherapp { namespace commands { namespace presentation {

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

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}}} // end of namespace launcherapp::commands::presentation

