#pragma once

#include "gui/SettingPage.h"

namespace soyokaze {
namespace commands {
namespace bookmarks {

class AppSettingBookmarkPage : public SettingPage
{
public:
	AppSettingBookmarkPage(CWnd* parentWnd);
	virtual ~AppSettingBookmarkPage();

	// 機能を利用するか
	BOOL mIsEnableBookmarks;

	// URLを絞り込み対象とするか?
	BOOL mIsUseURL;

	// Chromeの履歴検索を有効にする
	BOOL mIsEnableHistoryChrome;
	// Edgeの履歴検索を有効にする
	BOOL mIsEnableHistoryEdge;

	// クエリのタイムアウト(msec)
	int mTimeout;
	// 候補の件数上限
	int mCandidates;
	// Migemo検索を使用するか?
	BOOL mIsUseMigemo;
	// 履歴のURLを絞り込み対象にするか?
	BOOL mIsUseURLForHistory;


protected:
	bool UpdateStatus();

	BOOL OnKillActive() override;
	BOOL OnSetActive() override;
	void OnOK() override;
	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	void OnEnterSettings() override;
// 実装
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnCheckEnable();
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace soyokaze

