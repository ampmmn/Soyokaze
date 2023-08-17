#include "pch.h"
#include "WebSearchCommandParam.h"
#include "resource.h"

namespace soyokaze {
namespace commands {
namespace websearch {

CommandParam::CommandParam() :
	mIsEnableShortcut(false)
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

