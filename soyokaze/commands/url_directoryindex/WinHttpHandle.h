#pragma once

#include <winhttp.h>
#pragma comment (lib, "winhttp.lib")

class WinHttpHandle
{
public:
	WinHttpHandle(HINTERNET h) : mHandle(h) {}
	~WinHttpHandle()
 	{ 
		if (mHandle) {
			WinHttpCloseHandle(mHandle);
		}
	}

	operator HINTERNET() { return mHandle; }

	HINTERNET mHandle;
};

