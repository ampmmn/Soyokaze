#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace filter {

// 前段フィルタの種別
enum PREFILTERTYPE {
	// 子プロセスの出力
	FILTER_SUBPROCESS = 0,
	// クリップボードから取得
	FILTER_CLIPBOARD,
	// テキスト文字列
	FILTER_TEXT,
};

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

public:
	CString mName;
	CString mDescription;
	// カレントディレクトリ
	CString mDir;
	// パス
	CString mPath;
	// パラメータ
	CString mParameter;
	// 前段の処理の種類
	int mPreFilterType;
	// 候補のキャッシュ方法(0:しない 1:する(アプリ終了まで))
	int mCacheType;
	// 前段の文字コード(FILTER_SUBPROCESSのみ)
	int mPreFilterCodePage;
	// 後段の処理の種類
	int mPostFilterType;
	// 後段のコマンド(mPostFilterType=0の場合)
	CString mAfterCommandName;
	// 後段のファイルorURL(mPostFilterType=1の場合)
	CString mAfterFilePath;
	// 後段のコマンドに渡すパラメータ
	CString mAfterCommandParam;

	CommandHotKeyAttribute mHotKeyAttr;
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

