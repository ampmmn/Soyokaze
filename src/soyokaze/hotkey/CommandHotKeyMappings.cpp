#include "pch.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "hotkey/CommandHotKeyAttribute.h"
#include <vector>
#include <algorithm>

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

/**
  名前と属性の組み合わせが同一かどうかを比較する
  項目の並び順は無視し、同じ名前の項目が同じホットキー属性を持つかで判定する
  @param[in] rhs 比較対象
  @return true:同一  false:同一ではない
*/
bool CommandHotKeyMappings::operator == (const CommandHotKeyMappings& rhs) const
{
	// 件数が異なる場合は同一ではない
	if (in->mItems.size() != rhs.in->mItems.size()) {
		return false;
	}

	// 自身の各項目について、同じ名前の項目が相手にあり、属性も一致するかを確認する
	for (const auto& item : in->mItems) {
		auto itFind = std::find_if(rhs.in->mItems.begin(), rhs.in->mItems.end(), [&](const PImpl::ITEM& rhsItem) {
			return rhsItem.mName == item.mName;
		});
		if (itFind == rhs.in->mItems.end()) {
			return false;
		}
		if (itFind->mAttr != item.mAttr) {
			return false;
		}
	}
	return true;
}

bool CommandHotKeyMappings::operator != (const CommandHotKeyMappings& rhs) const
{
	return (*this == rhs) == false;
}


