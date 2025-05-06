#pragma once
#include "mainwindow/MainWindowCommandLauncher.h"

#include "commands/remote/RemoteClient.h"

namespace launcherapp { namespace commands { namespace remote {

class RemoteCommandLauncher : public launcherapp::mainwindow::CommandLauncher
{
public:
	RemoteCommandLauncher(RemoteClient* client);
	virtual ~RemoteCommandLauncher();

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

private:
	RemoteClient* mClient{nullptr};

};



}}} // end of namespace launcherapp::commands::remote

