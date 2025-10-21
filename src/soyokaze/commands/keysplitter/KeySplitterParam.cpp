#include "pch.h"
#include "KeySplitterParam.h"


namespace launcherapp {
namespace commands {
namespace keysplitter {

CommandParam::CommandParam()
{
}

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mMapping(rhs.mMapping)
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
		mMapping = rhs.mMapping;
	}
	return *this;
}

bool CommandParam::GetMapping(
		const ModifierState& state,
	 	ITEM& item
)
{
	auto it = mMapping.find(state);
	if (it == mMapping.end()) {
		return false;
	}

	item = it->second;
	return true;
}

void CommandParam::SetMapping(
		const ModifierState& state,
	 	const ITEM& item
)
{
	mMapping[state] = item;
}

bool CommandParam::EraseMapping(const ModifierState& state)
{
	auto it = mMapping.find(state);
	if (it == mMapping.end()) {
		return false;
	}
	mMapping.erase(it);
	return true;
}

bool CommandParam::IsStateExists(const ModifierState& state) const
{
	return mMapping.count(state) != 0;
}

bool CommandParam::IsEmptyMapping() const
{
	return mMapping.empty();
}

bool CommandParam::DeleteMapping(const ModifierState& state)
{
	auto it = mMapping.find(state); 
	if (it == mMapping.end()) {
		return false;
	}
	
	mMapping.erase(it);
	return true;
}


} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp



