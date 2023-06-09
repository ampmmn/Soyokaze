#pragma once

#include "HotKeyAttribute.h"

namespace soyokaze {
namespace commands {
namespace shellexecute {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	int GetShowType() const;
	void SetShowType(int type);
public:
	// コマンド名
	CString mName;
	// 説明
	CString mDescription;

	// 管理者権限で実行
	BOOL mIsRunAsAdmin;

	// 表示方法
	int mShowType;
	// カレントディレクトリ
	CString mDir;
	// パス
	CString mPath;
	// パラメータ
	CString mParameter;
	// パス(引数なし版)
	CString mPath0;
	// パラメータ(引数なし版)
	CString mParameter0;
	// 引数なし版を使うか?
	BOOL mIsUse0;

	// 引数が与えられなかった場合に引数入力用のダイアログを追加で表示する
	BOOL mIsShowArgDialog;

	HOTKEY_ATTR mHotKeyAttr;
	bool mIsGlobal;
};


}
}
}

