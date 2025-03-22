#pragma once

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryEventListener
{
public:
	virtual ~ClipboardHistoryEventListener() {}

	//! クリップボードが更新された
	virtual void UpdateClipboard(LPCTSTR data) = 0;
};


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

