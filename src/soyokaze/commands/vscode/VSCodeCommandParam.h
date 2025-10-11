#pragma once

#include "setting/Settings.h"

namespace launcherapp { namespace commands { namespace vscode {

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool Save(Settings& settings) const;
	bool Load(Settings& settings);

	CString GetVSCodeExePath() const;

	bool HasPrefix() const;

public:
	CString mPrefix;
	// 検索を有効にする最小文字数
	int mMinTriggerLength{5};
	// 機能を利用するか?
	bool mIsEnable{true};
	// フルパス表示
	bool mIsShowFullPath{true};
};


}}} // end of namespace launcherapp::commands::vscode

