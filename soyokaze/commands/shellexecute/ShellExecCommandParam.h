#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp {
namespace commands {
namespace shellexecute {

struct ATTRIBUTE {

	int GetShowType() const;

	CString mPath;
	CString mParam;
	CString mDir;
	int mShowType = 0;
};

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

public:
	// コマンド名
	CString mName;
	// 説明
	CString mDescription;

	ATTRIBUTE mNormalAttr;
	ATTRIBUTE mNoParamAttr;

	// 管理者権限で実行
	BOOL mIsRunAsAdmin;

	// 引数なし版を使うか?
	BOOL mIsUse0;

	// 引数が与えられなかった場合に引数入力用のダイアログを追加で表示する
	BOOL mIsShowArgDialog;

	// 説明欄の文字列をマッチングに利用するか?
	BOOL mIsUseDescriptionForMatching;

	// アイコンデータ(空の場合はデフォルトアイコンを使用)
	std::vector<uint8_t> mIconData;

	// ホットキー
	CommandHotKeyAttribute mHotKeyAttr;
};


}
}
}

