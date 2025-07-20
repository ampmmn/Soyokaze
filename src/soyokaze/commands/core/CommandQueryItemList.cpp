#include "pch.h"
#include "CommandQueryItemList.h"
#include "commands/core/CommandRanking.h"
#include "matcher/Pattern.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {

using CommandRanking = launcherapp::commands::core::CommandRanking;

struct CommandQueryItemList::PImpl
{
	std::vector<CommandQueryItem>& GetItems() {
		return mItems.empty() == false || mHiddenItems.empty() == false ? mItems : mWeakItems;
	}

	std::vector<CommandQueryItem> mItems;
	// 弱一致、非表示の要素は分けて管理する
	std::vector<CommandQueryItem> mWeakItems;
	std::vector<CommandQueryItem> mHiddenItems;
};

CommandQueryItemList::CommandQueryItemList() : in(new PImpl)
{
}

CommandQueryItemList::~CommandQueryItemList()
{
}

// 完全一致のものを探す
bool CommandQueryItemList::FindWholeMatchItem(Command** command)
{
	auto& items = in->GetItems();
	for (auto& item : items) {
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
	if (item.mMatchLevel == Pattern::WeakMatch) {
		in->mWeakItems.push_back(item);
	}
	else if (item.mMatchLevel == Pattern::HiddenMatch) {
		in->mHiddenItems.push_back(item);
	}
	else {
		in->mItems.push_back(item);
	}
}

size_t CommandQueryItemList::GetItemCount()
{
	return in->GetItems().size();
}

bool CommandQueryItemList::GetItem(size_t index, CommandQueryItem* retItem)
{
	ASSERT(retItem != nullptr);

	auto& items = in->GetItems();
	if (items.size() <= index) {
		return false;
	}

	auto& itemRef = items[index];
	*retItem = itemRef;

	return true;
}

void CommandQueryItemList::Sort()
{
	const CommandRanking* rankPtr = CommandRanking::GetInstance();

	auto& items = in->GetItems();
	std::stable_sort(items.begin(), items.end(),
		[rankPtr](const CommandQueryItem& l, const CommandQueryItem& r) {

			// 一致度レベルで比較
			// 一致度レベルの数値が高いほうを優先するため、 R < L で比較
			if (r.mMatchLevel < l.mMatchLevel) { return true; }
			if (r.mMatchLevel > l.mMatchLevel) { return false; }

			// 一致レベルが同じ場合は優先順位による判断を行う
			auto& cmdL = l.mCommand;
			auto& cmdR = r.mCommand;

			// 優先度の数値が高いほうを優先するため、 R < L で比較
			int priorityL = rankPtr->Get(cmdL.get());
			int priorityR = rankPtr->Get(cmdR.get());
			return priorityR < priorityL;

			// Note: 動的なコマンド(AdhocCommand)が生成したコマンド順序を維持したいので、
			//       名前での昇順ソートを行わないことにしている
			//       例: vmxのMRU、定型文グループの要素など
	});
}

}

