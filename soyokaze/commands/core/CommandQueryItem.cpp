#include "pch.h"
#include "CommandQueryItem.h"
#include "commands/core/CommandRanking.h"
#include "matcher/Pattern.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {

using CommandRanking = launcherapp::commands::core::CommandRanking;

static void ReleaseObject(void* ptr)
{
	auto cmd = (launcherapp::core::Command*)ptr;
	if (cmd) {
		cmd->Release();
	}
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

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct CommandQueryItemList::PImpl
{
	std::vector<CommandQueryItem> mItems;
};

CommandQueryItemList::CommandQueryItemList() : in(new PImpl)
{
}

CommandQueryItemList::~CommandQueryItemList()
{
}

bool CommandQueryItemList::IsEmpty() const
{
	return in->mItems.empty();
}

// 完全一致のものを探す
bool CommandQueryItemList::FindWholeMatchItem(Command** command)
{
	for (auto& item : in->mItems) {
		if (item.mMatchLevel != Pattern::WholeMatch) {
			continue;
		}

		*command = item.mCommand.get();
		(*command)->AddRef();

		return true;
	}
	return false;
}

void CommandQueryItemList::Add(const CommandQueryItem& item)
{
	in->mItems.push_back(item);
}

size_t CommandQueryItemList::GetItemCount()
{
	return in->mItems.size();
}

size_t CommandQueryItemList::GetItems(Command** array, size_t arrayLen)
{
	size_t copyCount = arrayLen > GetItemCount() ? GetItemCount() : arrayLen;
	for (size_t i = 0; i < copyCount; ++i) {
		auto& item = in->mItems[i];

		auto command = item.mCommand.get();
		command->AddRef();
		array[i] = command;
	}
	return copyCount;
}

void CommandQueryItemList::Sort()
{
	const CommandRanking* rankPtr = CommandRanking::GetInstance();

	auto& items = in->mItems;
	std::stable_sort(items.begin(), items.end(),
		[rankPtr](const CommandQueryItem& l, const CommandQueryItem& r) {
			if (r.mMatchLevel < l.mMatchLevel) { return true; }
			if (r.mMatchLevel > l.mMatchLevel) { return false; }

			// 一致レベルが同じ場合は優先順位による判断を行う
			auto& cmdL = l.mCommand;
			auto& cmdR = r.mCommand;

			auto nameL = cmdL->GetName();
			auto nameR = cmdR->GetName();

			int priorityL = cmdL->IsPriorityRankEnabled() ? rankPtr->Get(nameL) : 0;
			int priorityR = cmdR->IsPriorityRankEnabled() ? rankPtr->Get(nameR) : 0;
			return priorityR < priorityL;
	});
}


}

