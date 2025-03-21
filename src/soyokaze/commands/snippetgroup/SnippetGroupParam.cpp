#include "pch.h"
#include "SnippetGroupParam.h"
#include "commands/core/CommandEntryIF.h"
#include "utility/AES.h"

namespace launcherapp {
namespace commands {
namespace snippetgroup {

static LPCSTR PASSPHRASE = "nandokuka";

SnippetGroupParam::SnippetGroupParam()
{
}

SnippetGroupParam::~SnippetGroupParam()
{
}

bool SnippetGroupParam::operator == (const SnippetGroupParam& rhs) const
{
	if (&rhs == this) {
		return true;
	}

	return mName == rhs.mName &&
		mDescription == rhs.mDescription &&
		mItems == rhs.mItems &&
		mHotKeyAttr == rhs.mHotKeyAttr;
}

bool SnippetGroupParam::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	// 気休め程度の難読化をしておく
	utility::aes::AES aes;
	aes.SetPassphrase(PASSPHRASE);

	// Note: nameは上位で書き込みを行っているのでここではしない
	entry->Set(_T("description"), mDescription);

	int count = (int)mItems.size();
	entry->Set(_T("ItemCount"), count);

	CString keyName;
	for (int i = 0; i < count; ++i) {
		keyName.Format(_T("Item%d"), i+1);
		mItems[i].Save(entry, keyName, aes);
	}

	return true;
}

bool SnippetGroupParam::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	// 気休め程度の難読化をしておく
	utility::aes::AES aes;
	aes.SetPassphrase(PASSPHRASE);

	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	int count = entry->Get(_T("ItemCount"), 0);

	ItemList items;
	items.reserve(count);

	CString keyName;
	for (int i = 0; i < count; ++i) {
		keyName.Format(_T("Item%d"), i+1);
		Item item;
		if (item.Load(entry, keyName, aes) == false) {
			continue;
		}
		items.push_back(item);
	}

	mItems.swap(items);

	return true;
}

void SnippetGroupParam::swap(SnippetGroupParam& rhs)
{
	std::swap(mName, rhs.mName);
	std::swap(mDescription, rhs.mDescription);
	mItems.swap(rhs.mItems);
}

}
}
}



