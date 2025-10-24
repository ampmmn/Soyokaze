#include "pch.h"
#include "SnippetCommandParam.h"


namespace launcherapp { namespace commands { namespace snippet {

bool CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("text"), mText);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mText = entry->Get(_T("text"), _T(""));

	return true;
}

}}}
