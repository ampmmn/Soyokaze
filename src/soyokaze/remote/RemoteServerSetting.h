#pragma once

#include "setting/AppPreference.h"

namespace launcherapp { namespace remote {

class RemoteServerSetting
{
public:
	RemoteServerSetting() {
		auto pref = AppPreference::Get();
		mIsEnable = pref->IsEnableRemoteServer();
	}

	bool IsEnable() const {
		return mIsEnable;
	}
private:
	bool mIsEnable{false};
};



}} // end of namespace launcherapp::remote
