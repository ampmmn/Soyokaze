#include "pch.h"
#include "CommandQueryDefaultResult.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandQueryItem = launcherapp::CommandQueryItem;

namespace launcherapp { namespace commands { namespace core {


CommandQueryDefaultResult::CommandQueryDefaultResult()
{
}

CommandQueryDefaultResult::~CommandQueryDefaultResult()
{
}

CommandQueryDefaultResult* CommandQueryDefaultResult::Create()
{
	return new CommandQueryDefaultResult();
}

void CommandQueryDefaultResult::Add(const CommandQueryItem& item)
{
	mResultItems.push_back(item);
}

size_t CommandQueryDefaultResult::GetCount()
{
	return mResultItems.size();
}

bool CommandQueryDefaultResult::IsEmpty()
{
	return mResultItems.empty();
}

bool CommandQueryDefaultResult::Get(size_t index, launcherapp::core::Command** cmd, int* matchLevel)
{
	if (mResultItems.size() <= index) {
		return false;
	}

	*cmd = mResultItems[index].mCommand.get();
	(*cmd)->AddRef();

	if (matchLevel) {
		*matchLevel = mResultItems[index].mMatchLevel;
	}
	return true;
}

launcherapp::core::Command* CommandQueryDefaultResult::GetItem(size_t index, int* matchLevel)
{
	if (mResultItems.size() <= index) {
		return nullptr;
	}

	auto cmd = mResultItems[index].mCommand.get();
	cmd->AddRef();

	if (matchLevel) {
		*matchLevel = mResultItems[index].mMatchLevel;
	}

	return cmd;
}

uint32_t CommandQueryDefaultResult::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t CommandQueryDefaultResult::Release()
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

}}}


