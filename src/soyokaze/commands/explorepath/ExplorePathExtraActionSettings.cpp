#include "pch.h"
#include "ExplorePathExtraActionSettings.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace explorepath {

ExtraActionSettings::ExtraActionSettings()
{
}

ExtraActionSettings::~ExtraActionSettings()
{
}

void ExtraActionSettings::Save()
{
	auto pref = AppPreference::Get();
	auto& settings = (Settings&)pref->GetSettings();
	Save(&settings);
}

void ExtraActionSettings::Save(Settings* settingsPtr)
{
	CString key;

	int num_entries = (int)mEntries.size();

	settingsPtr->Set(_T("ExplorePath:NumberOfExtraActions"), num_entries);
	for (int i = 1; i <= num_entries; ++i) {

		const auto& entry = mEntries[i-1];

		key.Format(_T("ExplorePath:Label%d"), i);
		settingsPtr->Set(key, entry.mLabel);
		key.Format(_T("ExplorePath:Command%d"), i);
		settingsPtr->Set(key, entry.mCommand);

		key.Format(_T("ExplorePath:HotkeyVKCode%d"), i);
		settingsPtr->Set(key, (int)entry.mHotkeyAttr.GetVKCode()); 
		key.Format(_T("ExplorePath:HotkeyModifiers%d"), i);
		settingsPtr->Set(key, (int)entry.mHotkeyAttr.GetModifiers()); 

		key.Format(_T("ExplorePath:IsForFile%d"), i);
		settingsPtr->Set(key, entry.mIsForFile);
		key.Format(_T("ExplorePath:IsForFolder%d"), i);
		settingsPtr->Set(key, entry.mIsForFolder);
	}
}

void ExtraActionSettings::Load()
{
	auto pref = AppPreference::Get();
	auto& settings = pref->GetSettings();

	std::vector<Entry> entries;

	CString key;
	CString data;

	int num_entries = settings.Get(_T("ExplorePath:NumberOfExtraActions"), 0);
	for (int i = 1; i <= num_entries; ++i) {

		Entry entry;

		key.Format(_T("ExplorePath:Label%d"), i);
		entry.mLabel = settings.Get(key, _T(""));
		if (entry.mLabel.IsEmpty()) {
			continue;
		}
		key.Format(_T("ExplorePath:Command%d"), i);
		entry.mCommand = settings.Get(key, _T(""));
		if (entry.mCommand.IsEmpty()) {
			continue;
		}
		key.Format(_T("ExplorePath:HotkeyVKCode%d"), i);
		uint32_t vkCode = settings.Get(key, 0); 
		key.Format(_T("ExplorePath:HotkeyModifiers%d"), i);
		uint32_t modifiers = settings.Get(key, 0); 
		if (vkCode == 0) {
			continue;
		}
		entry.mHotkeyAttr = HOTKEY_ATTR(modifiers, vkCode);

		key.Format(_T("ExplorePath:IsForFile%d"), i);
		entry.mIsForFile = settings.Get(key, false);
		key.Format(_T("ExplorePath:IsForFolder%d"), i);
		entry.mIsForFolder = settings.Get(key, false);
		if (entry.mIsForFile == false && entry.mIsForFolder == false) {
			continue;
		}

		entries.push_back(entry);
	}

	mEntries.swap(entries);
}

int ExtraActionSettings::GetEntryCount()
{
	return (int)mEntries.size();
}

bool ExtraActionSettings::GetEntry(size_t index, ExtraActionSettings::Entry& entry)
{
	if (GetEntryCount() <= index) {
		return false;
	}

	entry = mEntries[index];
	return true;
}

void ExtraActionSettings::SwapEntries(std::vector<Entry>& entries)
{
	mEntries.swap(entries);
}


}}}

