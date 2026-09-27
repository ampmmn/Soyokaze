#include "pch.h"
#include "features/keywordmanager/CommandImportNameResolver.h"

CString launcherapp::core::CommandImportNameResolver::GetUniqueName(
	const CString& originalName,
	const std::function<bool(const CString&)>& isOccupied
)
{
	for (unsigned int suffix = 1; ; ++suffix) {
		CString candidateName;
		candidateName.Format(_T("%s-%u"), (LPCTSTR)originalName, suffix);
		if (isOccupied(candidateName) == false) {
			return candidateName;
		}
	}
}
