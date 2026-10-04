#include "pch.h"
#include "CandidateExclusionList.h"
#include "setting/AppPreference.h"
#include "utility/Regex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace common {

struct CandidateExclusionList::PImpl
{
	std::set<CString> mPaths;
	std::vector<launcherapp::utility::Regex> mDisplayNamePatterns;
};

CandidateExclusionList::CandidateExclusionList() : in(std::make_unique<PImpl>())
{
}

CandidateExclusionList::~CandidateExclusionList()
{
}

bool CandidateExclusionList::IsExcludedPath(const CString& path) const
{
	CString p(path);
	p.MakeLower();
	return in->mPaths.find(p) != in->mPaths.end();
}

bool CandidateExclusionList::IsExcludedDisplayName(const CString& displayName) const
{
	for (const auto& regex : in->mDisplayNamePatterns) {
		if (regex.PartialMatch(displayName)) {
			return true;
		}
	}
	return false;
}

void CandidateExclusionList::Load()
{
	auto pref = AppPreference::Get();
	auto& settings = pref->GetSettings();

	std::set<CString> paths;
	std::vector<launcherapp::utility::Regex> displayNamePatterns;

	TCHAR key[64];
	int n = settings.Get(_T("Soyokaze:ExcludePathCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("Soyokaze:ExcludePath%d"), index+1);
		CString path = settings.Get(key, _T(""));
		if (path.IsEmpty()) {
			continue;
		}
		path.MakeLower();
		paths.insert(path);
	}

	n = settings.Get(_T("ExcludeTarget:PatternCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("ExcludeTarget:Pattern%d"), index);
		CString pattern = settings.Get(key, _T(""));
		launcherapp::utility::Regex regex;
		if (regex.Compile(pattern, false) == false) {
			SPDLOG_ERROR(_T("Invalid display-name exclusion pattern: {}"), (LPCTSTR)pattern);
			continue;
		}
		displayNamePatterns.push_back(regex);
	}

	in->mPaths.swap(paths);
	in->mDisplayNamePatterns.swap(displayNamePatterns);
}

}
}
}
