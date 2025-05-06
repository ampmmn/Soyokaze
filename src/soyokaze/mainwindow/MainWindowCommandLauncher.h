#pragma once

#include "commands/core/CommandQueryRequest.h"
#include <vector>

namespace launcherapp { namespace mainwindow {

class CommandLauncher
{
public:

public:
	virtual ~CommandLauncher() {}

	// 読み込み
	virtual bool Load() = 0;
	// 検索リクエスト実施
	virtual void Query(const launcherapp::commands::core::CommandQueryRequest& req) = 0;
	// コマンド実行
	virtual bool Execute(const CString& str) = 0;
	// ファイルがドロップされた
	virtual void DropFiles(const std::vector<CString>& files) = 0;
	// URLがドロップされた
	virtual void DropURL(const CString& urlString) = 0;
	// ウインドウ
	virtual void CaptureWindow(HWND hwnd) = 0;

};



}} // end of namespace launcherapp::mainwindow

