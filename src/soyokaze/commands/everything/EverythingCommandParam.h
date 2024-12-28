#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace everything {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam&) = default;
	~CommandParam();

	CString BuildQueryString(const CString& queryStr);

public:
	CString mName;
	CString mDescription;

	// 検索対象ディレクトリ
	CString mBaseDir;
	// 検索対象
	int mTargetType = 0;   // 0:ファイルとフォルダ 1:ファイルのみ 2:フォルダのみ
	// 大文字小文字を区別する
	BOOL mIsMatchCase = FALSE;
	// 正規表現を使う
	BOOL mIsRegex = FALSE;
	// その他のパラメータ
	CString mOtherParam;
	// ホットキー設定
	CommandHotKeyAttribute mHotKeyAttr;
};


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

