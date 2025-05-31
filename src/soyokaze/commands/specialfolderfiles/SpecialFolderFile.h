#pragma once

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {

// 種別
enum {
	// 最近使ったファイル
	TYPE_RECENT,
	// スタートメニュー
	TYPE_STARTMENU,
};

struct ITEM
{
	// 名前
	CString mName;
	// フルパス
	CString mFullPath;
	// 説明
	CString mDescription;
	// ショートカットファイル
	CString mLinkPath;
	// 種別
	int mType{TYPE_RECENT};
	// 更新日時
	FILETIME mWriteTime{};
};

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

