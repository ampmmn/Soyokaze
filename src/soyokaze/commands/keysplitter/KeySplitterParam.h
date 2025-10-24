#pragma once

#include <vector>
#include <map>

#include "KeySplitterModifierState.h"
#include "commands/core/CommandEntryIF.h"

namespace launcherapp {
namespace commands {
namespace keysplitter {

struct ITEM
{
	CString mCommandName;
	CString mActionName;
};

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool Save(CommandEntryIF* entry);
	bool Load(CommandEntryIF* entry);

	bool GetMapping(const ModifierState& stae, ITEM& item);
	void SetMapping(const ModifierState& stae, const ITEM& item);
	bool EraseMapping(const ModifierState& state);
	bool IsStateExists(const ModifierState& state) const;
	bool IsEmptyMapping() const;
	bool DeleteMapping(const ModifierState& stae);


public:
	CString mName;
	CString mDescription;

	std::map<ModifierState, ITEM> mMapping;
};



} // end of namespace group
} // end of namespace commands
} // end of namespace launcherapp

