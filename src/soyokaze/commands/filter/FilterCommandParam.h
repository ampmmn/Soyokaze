#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp {
namespace commands {
namespace filter {

// 前段フィルタの種別
enum PREFILTERTYPE {
	// 子プロセスの出力
	FILTER_SUBPROCESS = 0,
	// クリップボードから取得
	FILTER_CLIPBOARD,
	// 固定値
	FILTER_CONSTANT,
};

// 後段フィルタの種別
enum POSTFILTERTYPE {
	POSTFILTER_COMMAND = 0,
	POSTFILTER_SUBPROCESS,
	POSTFILTER_CLIPBOARD,
};

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	int GetAfterShowType() const;

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	bool BuildCandidateTextRegExp(CString& errMsg);
	bool ReplaceCandidateText(const CString& input, CString& replacedText) const;

public:
	CString mName;
	CString mDescription;
	// (前段の)カレントディレクトリ
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
	// FILTER_CONSTANT時の固定値として用いる値
	CString mPreFilterText;
	// 後段の処理の種類
	int mPostFilterType;
	// 後段のコマンド(mPostFilterType=0の場合)
	CString mAfterCommandName;
	// 後段のファイルorURL(mPostFilterType=1の場合)
	CString mAfterFilePath;
	// 後段のコマンドに渡すパラメータ
	CString mAfterCommandParam;
	// 後段のカレントディレクトリ
	CString mAfterDir;
	// 後段のプログラムの表示方法
	int mAfterShowType;
	// 候補のテキストを置換するか?
	bool mIsReplaceText;
	// 置換パターン
	CString mReplacePattern;
	// 置換後の文字列
	CString mReplaceText;

	CommandHotKeyAttribute mHotKeyAttr;

	tregex mRegPattern;
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

