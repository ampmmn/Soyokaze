#pragma once

namespace launcherapp { namespace externaltool { namespace webbrowser {


class BrowserEnvironment
{
public:
	virtual ~BrowserEnvironment() = default;

	// 利用可能か?
	virtual bool IsAvailable() = 0;
	// 実行ファイルのパスを取得する
	virtual bool GetInstalledExePath(CString& path) = 0;
	// 実行パラメータを取得する
	virtual bool GetCommandlineParameter(CString& param) = 0;
	// ブックマークデータのパスを取得
	virtual bool GetBookmarkFilePath(CString& path) = 0;
	// 履歴ファイルのパスを取得
	virtual bool GetHistoryFilePath(CString& path) = 0;
	// 製品名を取得
	virtual bool GetProductName(CString& name) = 0;

};

}}}


