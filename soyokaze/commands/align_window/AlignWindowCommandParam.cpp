#include "pch.h"
#include "AlignWindowCommandParam.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace align_window {


CommandParam::CommandParam() :
	mIsGlobal(false),
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

