#pragma once

namespace launcherapp {
namespace commands {
namespace everything {

class CommandParam
{
public:
	CommandParam()
	{
	}
	CommandParam(const CommandParam&) = default;
	~CommandParam()
	{
	}

public:
	CString mName;
	CString mDescription;

	// 検索対象ディレクトリ
	CString mBaseDir;
	// 検索対象
	int mTargetType = 0;   // 0:ファイルとフォルダ 1:ファイルのみ 2:フォルダのみ
	// 大文字小文字を区別する
	BOOL mIsMatchCase = FALSE;
};


} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

