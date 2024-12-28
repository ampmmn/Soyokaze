#pragma once

namespace launcherapp {
namespace commands {
namespace specialfolderfiles {

enum {
	TYPE_RECENT,
	TYPE_STARTMENU,
};

struct ITEM
{
	static int GetTypeFromCSIDL(int csidl);

	CString mName;
	CString mFullPath;
	CString mDescription;
	int mType = TYPE_RECENT;
	FILETIME mWriteTime = {};
};

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace launcherapp

