#pragma once

namespace soyokaze {
namespace commands {
namespace simple_dict {

class SimpleDictParam
{
public:
	SimpleDictParam() : mActionType(2), mAfterCommandParam(_T("$value"))
	{}

	SimpleDictParam(const SimpleDictParam&) = default;

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
	// 後段の処理の種類
	int mActionType;
	// 後段のコマンド(mPostFilterType=0の場合)
	CString mAfterCommandName;
	// 後段のファイルorURL(mPostFilterType=1の場合)
	CString mAfterFilePath;
	// 後段のコマンドに渡すパラメータ
	CString mAfterCommandParam;
};

}
}
}

