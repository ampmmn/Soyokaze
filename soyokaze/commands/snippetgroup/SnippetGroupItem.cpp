#include "pch.h"
#include "SnippetGroupItem.h"
#include "commands/core/CommandEntryIF.h"


namespace launcherapp {
namespace commands {
namespace snippetgroup {

Item::Item()
{
}

Item::~Item()
{
}

bool Item::operator == (const Item& rhs) const
{
	return mName == rhs.mName && mDescription == rhs.mDescription && mText == rhs.mText;
}

bool Item::Save(CommandEntryIF* entry, LPCTSTR prefix)
{
	CString keyName;
	keyName.Format(_T("%s_Name"), prefix);
	entry->Set(keyName, mName);
	keyName.Format(_T("%s_Description"), prefix);
	entry->Set(keyName, mDescription);
	keyName.Format(_T("%s_Text"), prefix);
	entry->Set(keyName, mText);
	return true;
}

bool Item::Load(CommandEntryIF* entry, LPCTSTR prefix)
{
	CString keyName;
	keyName.Format(_T("%s_Name"), prefix);
	mName = entry->Get(keyName, _T(""));
	keyName.Format(_T("%s_Description"), prefix);
	mDescription = entry->Get(keyName, _T(""));
	keyName.Format(_T("%s_Text"), prefix);
	mText = entry->Get(keyName, _T(""));
	return true;
}

void Item::swap(Item& rhs)
{
	std::swap(mName, rhs.mName);
	std::swap(mDescription, rhs.mDescription);
	std::swap(mText, rhs.mText);
}

}
}
}

