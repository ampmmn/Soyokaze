#pragma once
#include "mainwindow/MainWindowCommandLauncher.h"

namespace launcherapp { namespace mainwindow {

class LocalCommandLauncher : public CommandLauncher
{
public:
	LocalCommandLauncher();
	virtual ~LocalCommandLauncher();

	// 読み込み
	bool Load() override;
	// 検索リクエスト実施
	void Query(const launcherapp::commands::core::CommandQueryRequest& req) override;
	// コマンド実行
	bool Execute(const CString& str) override;
	// ファイルがドロップされた
	void DropFiles(const std::vector<CString>& files) override;
	// URLがドロップされた
	void DropURL(const CString& urlString) override;
	// ウインドウ
	void CaptureWindow(HWND hwnd) override;

	static bool CallExecute(const CString& str);
	static void CallDropFiles(const std::vector<CString>& files);
	static void CallDropURL(const CString& urlString);
	static void CallCaptureWindow(HWND hwnd);

};



}} // end of namespace launcherapp::mainwindow

