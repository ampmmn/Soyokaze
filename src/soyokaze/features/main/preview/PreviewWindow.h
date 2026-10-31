#pragma once

namespace launcherapp {
namespace mainwindow {
namespace preview {

class PreviewWindow
{
public:
	virtual ~PreviewWindow() {}

	/**
	  プレビューウインドウを表示する
	*/
	virtual void Enter() = 0;
	/**
	  プレビューウインドウを非表示にする
	*/
	virtual void Leave() = 0;
	/**
	  プレビューウインドウのウインドウハンドルを取得する
	  @return ウインドウハンドル
	*/
	virtual HWND GetHwnd() = 0;
};

}
}
}
