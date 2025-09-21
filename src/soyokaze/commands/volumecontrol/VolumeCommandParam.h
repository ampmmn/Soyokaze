#pragma once

#include "hotkey/CommandHotKeyAttribute.h"

namespace launcherapp {
namespace commands {
namespace volumecontrol {

class CommandParam
{
public:
	CommandParam() {}
	~CommandParam() {}

	int GetVolumeLevel() const {
		return mVolume;
	}

public:
	CString mName;
	CString mDescription;

	// 音量を変更するか?
	bool mIsSetVolume{false};

	// 変更する場合の音量(絶対値の場合は0-100)
	int mVolume{0};
	// 音量の設定値は相対値か?
	bool mIsRelative{false};

	// ミュート(消音)状態の変更方法(0:変更しない 1:ミュート 2:非ミュート 3:トグル)
	int mMuteControl{0};

	CommandHotKeyAttribute mHotKeyAttr;

};


}
}
}

