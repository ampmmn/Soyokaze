#include "pch.h"
#include "WebHistoryCommandParam.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace webhistory {

CommandParam::CommandParam() : mTimeout(250), mLimit(20),
	mIsEnableHistoryEdge(true), mIsEnableHistoryChrome(true), mIsUseMigemo(true), mIsUseURL(false)
{
}

CommandParam::~CommandParam()
{
}

}
}
}

