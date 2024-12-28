#pragma once

#include <vector>
#include <map>

#include "KeySplitterModifierState.h"

namespace launcherapp {
namespace commands {
namespace keysplitter {

struct ITEM
{
	CString mCommandName;
};

class CommandParam
{
public:
	CommandParam();
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool GetMapping(const ModifierState& stae, ITEM& item);
	void SetMapping(const ModifierState& stae, const ITEM& item);
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

