#pragma once

namespace launcherapp {
namespace mainwindow {

class LauncherInputStatus
{
public:
	virtual ~LauncherInputStatus() {}

	virtual bool HasKeyword() = 0;
};

}
}


