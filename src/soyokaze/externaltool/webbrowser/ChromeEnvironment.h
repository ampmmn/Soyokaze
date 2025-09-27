#pragma once

#include "externaltool/webbrowser/BrowserEnvironment.h"

namespace launcherapp { namespace externaltool { namespace webbrowser {


class ChromeEnvironment : public BrowserEnvironment
{
	ChromeEnvironment() = default;
	~ChromeEnvironment() override = default;

public:
	static ChromeEnvironment* GetInstance();

	// Chromeがインストールされているか?
	bool IsAvailable() override;
	// インストールされたchrome.exeのパスを取得する
	bool GetInstalledExePath(CString& path) override;
	// 実行パラメータを取得する
	bool GetCommandlineParameter(CString& param) override;
	// ブックマークデータのパスを取得
	bool GetBookmarkFilePath(CString& path) override;
	// 履歴ファイルのパスを取得
	bool GetHistoryFilePath(CString& path) override;
	// 製品名を取得
	bool GetProductName(CString& name) override;

};

}}}

