#include "pch.h"
#include "CommandQueryItem.h"
#include "matcher/Pattern.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {

static void ReleaseObject(void* ptr)
{
	auto cmd = (launcherapp::core::Command*)ptr;
	if (cmd) {
		cmd->Release();
	}
}

CommandQueryItem::CommandQueryItem() : 
	mMatchLevel(Pattern::Mismatch), mCommand(nullptr, ReleaseObject)
{
}

CommandQueryItem::CommandQueryItem(
	int level,
	launcherapp::core::Command* cmd
) : 
	mMatchLevel(level), mCommand(cmd, ReleaseObject)
{
	// Note: このコンストラクタを呼び出す場合、mCommandに対する参照カウントのインクリメントは呼び出し側の責任で行う
}

CommandQueryItem::CommandQueryItem(
	const CommandQueryItem& rhs
) : 
	mMatchLevel(rhs.mMatchLevel), mCommand(rhs.mCommand.get(), ReleaseObject)
{
	if (mCommand.get()) {
		mCommand->AddRef();
	}
}

CommandQueryItem::~CommandQueryItem()
{
}


CommandQueryItem& CommandQueryItem::operator = (
	const CommandQueryItem& rhs
)
{
	if (&rhs != this) {
		mMatchLevel = rhs.mMatchLevel;
		mCommand.reset(rhs.mCommand.get());
		if (mCommand.get()) {
			mCommand->AddRef();
		}
	}
	return *this;
}

}

