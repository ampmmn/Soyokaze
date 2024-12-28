#include "pch.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "hotkey/CommandHotKeyAttribute.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct CommandHotKeyMappings::PImpl
{
	struct ITEM {
		CString mName;
		CommandHotKeyAttribute mAttr;
	};
	std::vector<ITEM> mItems;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandHotKeyMappings::CommandHotKeyMappings() : in(std::make_unique<PImpl>())
{
}

CommandHotKeyMappings::~CommandHotKeyMappings()
{
}

int CommandHotKeyMappings::GetItemCount() const
{
	return (int)in->mItems.size();
}

CString CommandHotKeyMappings::GetName(int index) const
{
	ASSERT(0 <= index && index < (int)in->mItems.size());
	return in->mItems[index].mName;
}

void CommandHotKeyMappings::GetHotKeyAttr(
	int index,
 	CommandHotKeyAttribute& hotKeyAttr
) const
{
	ASSERT(0 <= index && index < (int)in->mItems.size());
	hotKeyAttr = in->mItems[index].mAttr;
}

void CommandHotKeyMappings::AddItem(
	const CString& name,
	const CommandHotKeyAttribute& hotKeyAttr

)
{
	PImpl::ITEM item;
	item.mName = name;
	item.mAttr = hotKeyAttr;
	in->mItems.push_back(item);
}

bool CommandHotKeyMappings::RemoveItem(const CString& name)
{
	for (auto it = in->mItems.begin(); it != in->mItems.end(); ++it) {
		if (name != it->mName) {
			continue;
		}
		in->mItems.erase(it);
		return true;
	}
	return false;
}

// コマンド名から割り当てキーの表示用文字列を取得する
CString CommandHotKeyMappings::FindKeyMappingString(const CString& name) const
{
	for(const auto& item : in->mItems) {
		if (name != item.mName) {
			continue;
		}
		return item.mAttr.ToString();
	}
	return _T("");
}

void CommandHotKeyMappings::Swap(CommandHotKeyMappings& rhs)
{
	in->mItems.swap(rhs.in->mItems);
}


