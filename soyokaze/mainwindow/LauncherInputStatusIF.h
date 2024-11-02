#pragma once

namespace launcherapp {
namespace mainwindow {

class LauncherInput
{
public:
	virtual ~LauncherInput() {}

	virtual bool HasKeyword() = 0;
};

}
}


