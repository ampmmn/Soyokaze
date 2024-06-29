#include "pch.h"
#include "AlignWindowCommandParam.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace align_window {


CommandParam::CommandParam() :
	mIsNotifyIfWindowNotFound(FALSE),
	mIsKeepActiveWindow(TRUE)
{
}

CommandParam::~CommandParam()
{
}

}
}
}

