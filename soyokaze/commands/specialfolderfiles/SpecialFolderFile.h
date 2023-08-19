#pragma once

namespace soyokaze {
namespace commands {
namespace specialfolderfiles {

enum {
	TYPE_RECENT,
	TYPE_STARTMENU,
};

struct ITEM
{
	CString mName;
	CString mFullPath;
	CString mDescription;
	int mType;
};

} // end of namespace specialfolderfiles
} // end of namespace commands
} // end of namespace soyokaze

