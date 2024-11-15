#pragma once

class CommandEntryIF;

namespace launcherapp {
namespace commands {
namespace snippetgroup {

class Item
{
public:
	Item();
	Item(const Item&) = default;
	~Item();

	bool operator == (const Item& rhs) const;

	bool Save(CommandEntryIF* entry, LPCTSTR prefix);
	bool Load(CommandEntryIF* entry, LPCTSTR prefix);

	void swap(Item& rhs);
public:
	CString mName;
	CString mDescription;
	CString mText;
};

}
}
}

