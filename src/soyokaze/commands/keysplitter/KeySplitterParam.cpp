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

bool CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);

	CString key;

	for (int i = 0; i < 16; ++i) {

		ModifierState state(i);
		ITEM item;
		bool hasItem = GetMapping(state, item);
		key.Format(_T("Use%d"), i);
		entry->Set(key, hasItem);

		if (hasItem == false) {
			continue;
		}

		key.Format(_T("Command%d"), i);
		entry->Set(key, item.mCommandName);
		key.Format(_T("ActionName%d"), i);
		entry->Set(key, item.mActionName);
	}

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	CString key;
	for (int i = 0; i < 16; ++i) {

		ModifierState state(i);
		key.Format(_T("Use%d"), i);
		bool hasEntry = entry->Get(key, false);

		if (hasEntry == false) {
			DeleteMapping(state);
			continue;
		}

		ITEM item;
		 

		key.Format(_T("Command%d"), i);
		item.mCommandName = entry->Get(key, _T(""));
		key.Format(_T("ActionName%d"), i);
		item.mActionName = entry->Get(key, _T(""));

		SetMapping(state, item);
	}

	return true;
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



