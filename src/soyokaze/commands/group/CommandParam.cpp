#include "pch.h"
#include "CommandParam.h"
#include "hotkey/CommandHotKeyManager.h"

namespace launcherapp {
namespace commands {
namespace group {

namespace {

CString GetItemTypeString(GroupItemType type)
{
	switch (type) {
	case GroupItemType::Path:
		return _T("Path");
	case GroupItemType::URL:
		return _T("URL");
	case GroupItemType::Command:
	default:
		return _T("Command");
	}
}

GroupItemType GetItemType(LPCTSTR value)
{
	if (CString(value).CompareNoCase(_T("Path")) == 0) {
		return GroupItemType::Path;
	}
	if (CString(value).CompareNoCase(_T("URL")) == 0) {
		return GroupItemType::URL;
	}
	return GroupItemType::Command;
}

}

CommandParam::CommandParam() : 
	mIsPassParam(false),
	mIsRepeat(false),
	mRepeats(1),
	mIsConfirm(true),
	mIsAllowAutoExecute(false)
{
}

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mItems(rhs.mItems),
	mIsPassParam(rhs.mIsPassParam),
	mIsRepeat(rhs.mIsRepeat),
	mRepeats(rhs.mRepeats),
	mIsConfirm(rhs.mIsConfirm),
	mIsAllowAutoExecute(rhs.mIsAllowAutoExecute),
	mHotKeyAttr(rhs.mHotKeyAttr)
{

}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (
	const CommandParam& rhs
)
{
	if (&rhs != this) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mItems = rhs.mItems;
		mIsPassParam = rhs.mIsPassParam;
		mIsRepeat = rhs.mIsRepeat;
		mRepeats = rhs.mRepeats;
		mIsConfirm = rhs.mIsConfirm;
		mIsAllowAutoExecute = rhs.mIsAllowAutoExecute;
		mHotKeyAttr = rhs.mHotKeyAttr;
	}
	return *this;
}

bool CommandParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("Description"), mDescription);
	entry->Set(_T("IsPassParam"), mIsPassParam);
	entry->Set(_T("IsRepeat"), mIsRepeat);
	entry->Set(_T("Repeats"), mRepeats);
	entry->Set(_T("IsConfirm"), mIsConfirm);
	entry->Set(_T("IsAllowAutoExecute"), mIsAllowAutoExecute);

	entry->Set(_T("CommandCount"), (int)mItems.size());
	int index = 1;

	TCHAR key[128];
	for (auto& item : mItems) {

		_stprintf_s(key, _T("ItemName%d"), index);
		entry->Set(key, item.mItemName);
		_stprintf_s(key, _T("ItemType%d"), index);
		entry->Set(key, GetItemTypeString(item.mType));
		_stprintf_s(key, _T("ItemParam%d"), index);
		entry->Set(key, item.mParam);
		_stprintf_s(key, _T("ShowType%d"), index);
		entry->Set(key, item.mShowType);
		_stprintf_s(key, _T("WorkDir%d"), index);
		entry->Set(key, item.mWorkDir);
		_stprintf_s(key, _T("IsWait%d"), index);
		entry->Set(key, item.mType != GroupItemType::URL && item.mIsWait);

		index++;
	}

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("Description"), _T(""));
	mIsPassParam = entry->Get(_T("IsPassParam"), false);
	mIsRepeat = entry->Get(_T("IsRepeat"), false);
	mRepeats = entry->Get(_T("Repeats"), 1);
	mIsConfirm = entry->Get(_T("IsConfirm"), true);
	mIsAllowAutoExecute = entry->Get(_T("IsAllowAutoExecute"), false);

	int nItems = entry->Get(_T("CommandCount"), 0);
	if (nItems > 32) {  // 32を上限とする
		nItems = 32;
	}

	std::vector<GroupItem> items;

	TCHAR key[128];
	for (int i = 1; i <= nItems; ++i) {

		GroupItem item;

		_stprintf_s(key, _T("ItemName%d"), i);
		item.mItemName = entry->Get(key, _T(""));
		_stprintf_s(key, _T("ItemType%d"), i);
		item.mType = GetItemType(entry->Get(key, _T("Command")));
		_stprintf_s(key, _T("ItemParam%d"), i);
		item.mParam = entry->Get(key, _T(""));
		_stprintf_s(key, _T("ShowType%d"), i);
		item.mShowType = entry->Get(key, SW_SHOW);
		_stprintf_s(key, _T("WorkDir%d"), i);
		item.mWorkDir = entry->Get(key, _T(""));
		_stprintf_s(key, _T("IsWait%d"), i);
		item.mIsWait = item.mType != GroupItemType::URL && entry->Get(key, false);

		items.push_back(item);
	}
	mItems.swap(items);

	// 旧設定では全要素に同じ引数を渡していたため、個別引数へ変換する
	if (mIsPassParam) {
		for (auto& item : mItems) {
			item.mParam = _T("$*");
		}
	}

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 

	return true;
}

} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp



