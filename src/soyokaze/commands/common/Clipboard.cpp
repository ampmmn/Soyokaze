#include "pch.h"
#include "Clipboard.h"
#include "utility/GlobalAllocMemory.h"
#include "mainwindow/controller/MainWindowController.h"

namespace launcherapp {
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
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->SetClipboardString(mem, &isSet);

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
