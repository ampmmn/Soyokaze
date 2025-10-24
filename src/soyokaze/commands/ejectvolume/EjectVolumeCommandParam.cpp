#include "pch.h"
#include "EjectVolumeCommandParam.h"

namespace launcherapp { namespace commands { namespace ejectvolume {

bool CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("DriveLetter"), (int)mDriveLetter);
	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	TCHAR letter = (TCHAR)entry->Get(_T("DriveLetter"), 0);
	if (letter < _T('A') || _T('Z') < letter) {
		return false;
	}
	mDriveLetter = letter;
	return true;
}

}}}
