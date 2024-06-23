#pragma once


namespace launcherapp {
namespace commands {
namespace watchpath {

class WatchTarget
{
public:
	virtual ~WatchTarget() {}

	virtual bool IsUpdated() = 0;

	virtual CString GetCommandName() = 0;
	virtual CString GetTitle() = 0;
	virtual CString GetDetail() = 0;

};

}
}
}

