#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

class CommandEntryIF;

namespace launcherapp {

namespace commands {
namespace simple_dict {

class SimpleDictParam
{
public:
	SimpleDictParam();
	SimpleDictParam(const SimpleDictParam&) = default;
	~SimpleDictParam();

	bool operator == (const SimpleDictParam& rhs) const;

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	void swap(SimpleDictParam& rhs);
public:
	CString mName;
	CString mDescription;

	// ファイルのパス
	CString mFilePath;
	// シート名
	CString mSheetName;
	// 範囲
	CString mRangeFront;
	CString mRangeBack;
	// 先頭行はヘッダか?
	BOOL mIsFirstRowHeader; 
	// コマンド名を入力しなくても候補を表示する
	BOOL mIsMatchWithoutKeyword;
	// 逆引きを有効にする
	BOOL mIsEnableReverse;
	// 更新を通知する
	BOOL mIsNotifyUpdate;
	// マクロを展開する
	BOOL mIsExpandMacro;
	// 後段の処理の種類
	int mActionType;
	// 後段のコマンド(mPostFilterType=0の場合)
	CString mAfterCommandName;
	// 後段のファイルorURL(mPostFilterType=1の場合)
	CString mAfterFilePath;
	// 後段のコマンドに渡すパラメータ
	CString mAfterCommandParam;

	CommandHotKeyAttribute mHotKeyAttr;
};

}
}
}

