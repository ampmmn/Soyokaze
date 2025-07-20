#include "pch.h"
#include "AlignWindowCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace align_window {


CommandParam::CommandParam() :
	mIsNotifyIfWindowNotFound(false),
	mIsKeepActiveWindow(true),
	mIsAllowAutoExecute(false)
{
}

CommandParam::~CommandParam()
{
}

bool CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);

	entry->Set(_T("IsNotifyIfWindowNotExist"), mIsNotifyIfWindowNotFound);
	entry->Set(_T("IsKeepActiveWindow"), mIsKeepActiveWindow);
	entry->Set(_T("IsAllowAutoExec"), mIsAllowAutoExecute);
	entry->Set(_T("ItemCount"), (int)mItems.size());

	CString key;

	int index = 1;
	for (auto& item : mItems) {

		key.Format(_T("CaptionStr%d"), index);
		entry->Set(key, item.mCaptionStr);
		key.Format(_T("ClassStr%d"), index);
		entry->Set(key, item.mClassStr);
		key.Format(_T("IsUseRegExp%d"), index);
		entry->Set(key, item.mIsUseRegExp != FALSE);
		key.Format(_T("IsApplyAll%d"), index);
		entry->Set(key, item.mIsApplyAll != FALSE);

		key.Format(_T("Action%d"), index);
		entry->Set(key, item.mAction);

		key.Format(_T("Placement%d"), index);
		entry->SetBytes(key, (uint8_t*)&item.mPlacement, sizeof(item.mPlacement));

		index++;
	}

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	CString name = entry->GetName();
	CString descriptionStr = entry->Get(_T("description"), _T(""));

	bool isNotify = entry->Get(_T("IsNotifyIfWindowNotExist"), false);
	bool isKeepActive = entry->Get(_T("IsKeepActiveWindow"), true);
	bool isAllowAutoExec = entry->Get(_T("IsAllowAutoExec"), false);

	CString key;

	std::vector<CommandParam::ITEM> items;

	int itemCount = entry->Get(_T("ItemCount"), 0);
	for (int i = 1; i <= itemCount; ++i) {

		CommandParam::ITEM item;

		key.Format(_T("CaptionStr%d"), i);
		item.mCaptionStr = entry->Get(key, _T(""));
		key.Format(_T("ClassStr%d"), i);
		item.mClassStr = entry->Get(key, _T(""));
		key.Format(_T("IsUseRegExp%d"), i);
		item.mIsUseRegExp = entry->Get(key, false) ? TRUE : FALSE;
		key.Format(_T("IsApplyAll%d"), i);
		item.mIsApplyAll = entry->Get(key, false) ? TRUE : FALSE;

		key.Format(_T("Action%d"), i);
		item.mAction = entry->Get(key, 0);

		key.Format(_T("Placement%d"), i);
		if (entry->GetBytes(key, (uint8_t*)&item.mPlacement, sizeof(WINDOWPLACEMENT)) == false) {
			continue;
		}
		item.BuildRegExp();

		items.push_back(item);
	}

	mName = name;
	mDescription = descriptionStr;
	mItems.swap(items);
	mIsNotifyIfWindowNotFound = isNotify;
	mIsKeepActiveWindow = isKeepActive;
	mIsAllowAutoExecute = isAllowAutoExec;

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 

	return true;
}

}
}
}

