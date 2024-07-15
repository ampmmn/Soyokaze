#include "pch.h"
#include "BookmarkCommandParam.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace bookmarks {

CommandParam::CommandParam() : 
	mIsEnableEdge(true), mIsEnableChrome(true), mIsUseURL(false)
{
}

CommandParam::~CommandParam()
{
}

}
}
}

