// あ
#pragma once

class CommandEntryIF;

namespace launcherapp {
namespace commands {
namespace watchpath {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam&) = default;
	~CommandParam();

	bool operator == (const CommandParam& rhs) const;

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	void swap(CommandParam& rhs);

public:
	// コマンド名
	CString mName;
	// 説明
	CString mDescription;
	// 監視対象パス
	CString mPath;
	// 更新検知時の通知メッセージ
	CString mNotifyMessage;
	// 更新検知後の間隔(秒単位)
	// 指定秒数が経過するまで次の通知をしない
	UINT mWatchInterval;
	// 除外フィルタ(,区切り、ワイルドカード指定)
	CString mExcludeFilter;
	// 無効にする
	bool mIsDisabled;
};


}
}
}

