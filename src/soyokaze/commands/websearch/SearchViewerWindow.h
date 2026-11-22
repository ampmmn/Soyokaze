#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace websearch {

class SearchViewerWindow : public CWnd
{
public:
	/** コマンド名に対応する検索ウインドウを作成する */
	SearchViewerWindow(const CString& commandName);
	virtual ~SearchViewerWindow();

	/** 検索ウインドウを作成または再利用してURLを表示する */
	bool Open(const CString& title, const CString& url);
	/** ウインドウのタイトルを更新する */
	void SetTitle(const CString& title);
	/** ウインドウを閉じて位置情報を保存する */
	void Close();

	/** 表示可能なHTTPまたはHTTPSのURLか確認する */
	static bool IsAllowedUrl(const CString& url);
	/** 保存済みの最大化状態に応じた表示コマンドを返す */
	static int GetRestoreShowCommand(const WINDOWPLACEMENT& placement);
	/** コマンド名からASCIIのウインドウ位置キーを生成する */
	static CString CreateWindowPlacementName(const CString& commandName);

protected:
	struct PImpl;
	std::shared_ptr<PImpl> in;

	DECLARE_DYNAMIC(SearchViewerWindow)
	DECLARE_MESSAGE_MAP()
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg LRESULT OnNavigatePending(WPARAM wParam, LPARAM lParam);

	/** WebView2を初期化してイベントを登録する */
	void InitializeWebView();
	/** アドレス欄へタイトルバー用フォントを設定する */
	void ConfigureAddressBarFont();
	/** アドレス欄とWebView2の表示領域を配置する */
	void Layout(int width, int height);
	/** 現在のウインドウ位置を保存する */
	void SaveWindowPosition();
};

}
}
}
