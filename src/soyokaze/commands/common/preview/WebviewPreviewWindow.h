#pragma once

#include "features/main/preview/PreviewWindow.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace common {
namespace preview {

class WebviewPreviewWindow : public launcherapp::mainwindow::preview::PreviewWindow
{
private:
	/**
	  プレビューウインドウを初期化する
	*/
	WebviewPreviewWindow();
	/**
	  ブラウザーの内部状態を解放する
	*/
	~WebviewPreviewWindow() override;

public:
	/**
	  共有プレビューウインドウを取得する
	  @return 共有インスタンス
	*/
	static WebviewPreviewWindow* Get();
	/**
	  指定されたHTMLファイルを表示する
	  @param[in] url ファイルのURL
	*/
	void SetFileUrl(const CString& url);
	/**
	  現在のページを空ページへ戻す
	*/
	void Clear();
	/**
	  プレビューウインドウを表示する
	*/
	void Enter() override;
	/**
	  プレビューウインドウを非表示にする
	*/
	void Leave() override;
	/**
	  プレビューウインドウのハンドルを取得する
	  @return ウインドウハンドル
	*/
	HWND GetHwnd() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
}
