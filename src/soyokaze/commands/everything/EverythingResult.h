#pragma once


namespace launcherapp {
namespace commands {
namespace everything {

class EverythingResult
{
public:
	EverythingResult() {}
	EverythingResult(LPCTSTR fullPath) : mFullPath(fullPath) {}

	CString mFullPath;
};

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

