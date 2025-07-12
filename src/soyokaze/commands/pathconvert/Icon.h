#pragma once

namespace launcherapp { namespace commands { namespace pathconvert {


class Icon
{
public:
	~Icon() {
		Reset();
	}

	void Reset() {
		if (mHandle) {
			DestroyIcon(mHandle);
			mHandle = nullptr;
		}
	}

	HICON Load(const CString& filePath) {
		if (mHandle == nullptr) {
			SHFILEINFO sfi = {};
			SHGetFileInfo(filePath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON);
			mHandle = sfi.hIcon;
		}
		return mHandle;
	}


private:
	HICON mHandle{nullptr};
};


}}}


