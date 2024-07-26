#pragma once

namespace launcherapp {
namespace commands {
namespace mmc {

class MMCSnapin
{
public:
	MMCSnapin() : mIconIndex(0)
	{
	}

	CString mDisplayName;
	CString mDescription;
	CString mMscFilePath;
	CString mIconFilePath;
	int mIconIndex;

};



} // end of namespace mmc
} // end of namespace commands
} // end of namespace launcherapp

