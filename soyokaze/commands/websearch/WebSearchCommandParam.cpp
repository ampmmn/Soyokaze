#include "pch.h"
#include "WebSearchCommandParam.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace websearch {

CommandParam::CommandParam() :
	mIsEnableShortcut(false), mIsGlobal(false)
{
}

CommandParam::~CommandParam()
{
}

bool CommandParam::IsEnableShortcutSearch() const
{
	return mIsEnableShortcut;
}

}
}
}

