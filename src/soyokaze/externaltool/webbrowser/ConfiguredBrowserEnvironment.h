#pragma once

#include "externaltool/webbrowser/BrowserEnvironment.h"
#include <memory>

namespace launcherapp { namespace externaltool { namespace webbrowser {

// アプリ設定の 外部ツール > Webブラウザ で設定したブラウザの情報を参照する
class ConfiguredBrowserEnvironment : public BrowserEnvironment
{
	ConfiguredBrowserEnvironment();
	~ConfiguredBrowserEnvironment() override;

public:
	static ConfiguredBrowserEnvironment* GetInstance();

	void Load();
	bool ShouldUseThisFor(const CString& url);

	// 外部ツールで設定したWebブラウザは利用可能か?
	bool IsAvailable() override;
	// 実行ファイルのパスを取得する
	bool GetInstalledExePath(CString& path) override;
	// 実行パラメータを取得する
	bool GetCommandlineParameter(CString& param) override;
	// ブックマークデータのパスを取得
	bool GetBookmarkFilePath(CString& path) override;
	// 履歴ファイルのパスを取得
	bool GetHistoryFilePath(CString& path) override;
	// 製品名を取得
	bool GetProductName(CString& name) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}}}

