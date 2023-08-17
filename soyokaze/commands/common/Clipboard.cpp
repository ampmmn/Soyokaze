#include "pch.h"
#include "Clipboard.h"
#include "utility/GlobalAllocMemory.h"
#include "SharedHwnd.h"

namespace soyokaze {
namespace commands {
namespace common {


bool Clipboard::Copy(const CString& text)
{
	// クリップボードにコピー
	size_t bufLen = sizeof(TCHAR) * (text.GetLength() + 1);
	GlobalAllocMemory mem(bufLen);
	_tcscpy_s((LPTSTR)mem.Lock(), text.GetLength() + 1, (LPCTSTR)text);
	mem.Unlock();

	BOOL isSet=FALSE;
	SharedHwnd sharedWnd;
	SendMessage(sharedWnd.GetHwnd(), WM_APP + 9, (WPARAM)&isSet, (LPARAM)(HGLOBAL)mem);

	if (isSet) {
		// セットされた場合はクリップボードが所有権を持つため、
		// GlobalAllocMemoryオブジェクト側の所有権を放棄する(解放しない)
		mem.Release();
	}

	return isSet;
}

}
}
}
