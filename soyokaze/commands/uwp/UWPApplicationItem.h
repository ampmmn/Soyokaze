#pragma once

#include <memory>

namespace soyokaze {
namespace commands {
namespace uwp {

struct ITEM
{
	ITEM(const CString& name, const CString& appId, HICON icon) :
		mName(name), mAppID(appId), mIcon(icon)
	{
	}
	ITEM(const ITEM&) = delete;

	~ITEM() {
		if (mIcon) {
			DestroyIcon(mIcon);
			TRACE(_T("Icon destroyed!\n"));
		}
	}

	ITEM& operator = (const ITEM&) = delete;

	CString mName;
	CString mAppID;
	HICON mIcon;
};

using ItemPtr = std::shared_ptr<ITEM>;

} // end of namespace uwp
} // end of namespace commands
} // end of namespace soyokaze

