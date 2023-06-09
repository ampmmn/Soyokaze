#include "pch.h"
#include "CommandHotKeyMappings.h"
#include "HotKeyAttribute.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


struct CommandHotKeyMappings::PImpl
{
	struct ITEM {
		CString mName;
		HOTKEY_ATTR mAttr;
		bool mIsGlobal;

	};
	std::vector<ITEM> mItems;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandHotKeyMappings::CommandHotKeyMappings() : in(new PImpl)
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
 	HOTKEY_ATTR& hotKeyAttr
) const
{
	ASSERT(0 <= index && index < (int)in->mItems.size());
	hotKeyAttr = in->mItems[index].mAttr;
}

bool CommandHotKeyMappings::IsGlobal(int index) const
{
	ASSERT(0 <= index && index < (int)in->mItems.size());
	return in->mItems[index].mIsGlobal;
}

void CommandHotKeyMappings::AddItem(
	const CString& name,
	const HOTKEY_ATTR& hotKeyAttr,
	bool isGlobal

)
{
	PImpl::ITEM item;
	item.mName = name;
	item.mAttr = hotKeyAttr;
	item.mIsGlobal = isGlobal;
	in->mItems.push_back(item);
}

void CommandHotKeyMappings::RemoveItem(const HOTKEY_ATTR& hotKeyAttr)
{
	if (hotKeyAttr.IsValid() == false) {
		return;
	}

	for (auto it = in->mItems.begin(); it != in->mItems.end(); ++it) {
		auto& attr = it->mAttr;
		if (attr == hotKeyAttr) {
			in->mItems.erase(it);
			return;
		}
	}
}

void CommandHotKeyMappings::Swap(CommandHotKeyMappings& rhs)
{
	in->mItems.swap(rhs.in->mItems);
}


