#pragma once

#include "commands/snippetgroup/SnippetGroupItem.h"
#include "hotkey/CommandHotKeyAttribute.h"
#include <vector>

class CommandEntryIF;

namespace launcherapp {

namespace commands {
namespace snippetgroup {

class SnippetGroupParam
{
	using ItemList = std::vector<Item>;
public:
	SnippetGroupParam();
	SnippetGroupParam(const SnippetGroupParam&) = default;
	~SnippetGroupParam();

	bool operator == (const SnippetGroupParam& rhs) const;

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	void swap(SnippetGroupParam& rhs);
public:
	CString mName;
	CString mDescription;

	ItemList mItems;
	CommandHotKeyAttribute mHotKeyAttr;
};

}
}
}

