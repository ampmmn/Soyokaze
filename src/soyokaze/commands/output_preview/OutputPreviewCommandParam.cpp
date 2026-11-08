#include "pch.h"
#include "OutputPreviewCommandParam.h"

namespace launcherapp {
namespace commands {
namespace output_preview {

OutputPreviewParam::OutputPreviewParam() : mExtension(_T(".txt"))
{
}

bool OutputPreviewParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("filepath"), mFilePath);
	entry->Set(_T("parameter"), mParameter);
	entry->Set(_T("workdir"), mWorkDir);
	entry->Set(_T("showtype"), mShowType);
	entry->Set(_T("resulttype"), mResultType);
	entry->Set(_T("extension"), mExtension);
	entry->Set(_T("previewtype"), mPreviewType);
	entry->Set(_T("timeout"), mTimeout);
	entry->Set(_T("force"), mForce);
	return true;
}

bool OutputPreviewParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mFilePath = entry->Get(_T("filepath"), _T(""));
	mParameter = entry->Get(_T("parameter"), _T(""));
	mWorkDir = entry->Get(_T("workdir"), _T(""));
	mShowType = entry->Get(_T("showtype"), 0);
	mResultType = entry->Get(_T("resulttype"), 0);
	mExtension = entry->Get(_T("extension"), _T(".txt"));
	mPreviewType = entry->Get(_T("previewtype"), 0);
	mTimeout = entry->Get(_T("timeout"), 500);
	mForce = entry->Get(_T("force"), 0);
	return true;
}

int OutputPreviewParam::GetShowType() const
{
	switch (mShowType) {
	case 1:
		return SW_HIDE;
	case 2:
		return SW_SHOWMINIMIZED;
	case 3:
		return SW_SHOWMAXIMIZED;
	default:
		return SW_SHOWNORMAL;
	}
}

bool OutputPreviewParam::ShouldShowResultForExitCode(DWORD exitCode) const
{
	return exitCode == 0 || mForce != 0;
}

}
}
}
