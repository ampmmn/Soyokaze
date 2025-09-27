#pragma once

#include "externaltool/webbrowser/BrowserEnvironment.h"

namespace launcherapp { namespace externaltool { namespace webbrowser {


class EdgeEnvironment : public BrowserEnvironment
{
	EdgeEnvironment() = default;
	~EdgeEnvironment() override = default;

public:
	static EdgeEnvironment* GetInstance();

	// 利用可能か?
	bool IsAvailable() override;
	// インストールされたmsedge.exeのパスを取得する
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

