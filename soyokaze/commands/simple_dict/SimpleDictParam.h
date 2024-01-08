#pragma once

namespace soyokaze {
namespace commands {
namespace simple_dict {

class SimpleDictParam
{
public:
	CString mName;
	CString mDescription;

	// ファイルのパス
	CString mFilePath;
	// シート名
	CString mSheetName;
	// 範囲
	CString mRange;
	// 先頭行はヘッダか?
	BOOL mIsFirstRowHeader; 
};

}
}
}

