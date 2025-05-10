#pragma once

namespace launcherapp { namespace commands { namespace common {

class ExecutablePath
{
public:
	ExecutablePath(const CString& path);
	~ExecutablePath();

	bool IsExecutable() const;

private:
	CString mPath;
};

}}} // end of namespace launcherapp::commands::common;

