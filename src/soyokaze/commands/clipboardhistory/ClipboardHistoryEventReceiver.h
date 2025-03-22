#pragma once

#include <memory>
#include "commands/clipboardhistory/ClipboardHistoryEventListener.h"

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryEventReceiver
{
public:
	ClipboardHistoryEventReceiver();
	~ClipboardHistoryEventReceiver();

public:
	bool Initialize();
	bool Activate(int interval, const CString& excludePattern);
	void Deactivate();
	void AddListener(ClipboardHistoryEventListener* listener);

private:
	static LRESULT CALLBACK OnWindowProc(HWND,UINT,WPARAM,LPARAM);
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

