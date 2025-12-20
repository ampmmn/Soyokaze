#pragma once

namespace launcherapp { namespace commands { namespace common {

class ExecutablePath
{
public:
	ExecutablePath(const CString& path);
	~ExecutablePath();

	void EnableMacros(bool enable);

	bool IsExecutable(bool includeRelative = true) const;
	bool IsDirectory() const;

private:
	CString mPath;
	bool mIsEnableMacros{true};
};

}}} // end of namespace launcherapp::commands::common;

