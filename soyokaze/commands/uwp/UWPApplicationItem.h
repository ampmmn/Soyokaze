#pragma once

#include <memory>

namespace launcherapp {
namespace commands {
namespace uwp {

struct ITEM
{
	ITEM(bool isUWP, const CString& name, const CString& appId, HICON icon) :
		mName(name), mAppID(appId), mIcon(icon), mIsUWP(isUWP)
	{
	}
	ITEM(const ITEM&) = delete;

	~ITEM() {
		if (mIcon) {
			DestroyIcon(mIcon);
		}
	}

	ITEM& operator = (const ITEM&) = delete;

	CString mName;
	CString mAppID;
	HICON mIcon;
	bool mIsUWP;
};

using ItemPtr = std::shared_ptr<ITEM>;

} // end of namespace uwp
} // end of namespace commands
} // end of namespace launcherapp

