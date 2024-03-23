#include "pch.h"
#include "ExcludePathList.h"
#include "setting/AppPreference.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace pathfind {


ExcludePathList::ExcludePathList() 
{
}

ExcludePathList::~ExcludePathList()
{
}

bool ExcludePathList::Contains(const CString& path)
{
	CString p(path);
	p.MakeLower();
	return mFiles.find(p) != mFiles.end();

}

void ExcludePathList::Load()
{
	auto pref = AppPreference::Get();
	auto& settings = pref->GetSettings();

	std::set<CString> files;

	TCHAR key[64];
	int n = settings.Get(_T("Soyokaze:ExcludePathCount"), 0);
	for (int index = 0; index < n; ++index) {
		_stprintf_s(key, _T("Soyokaze:ExcludePath%d"), index+1);
		CString path = settings.Get(key, _T(""));
		path.MakeLower();
		files.insert(path);
	}

	mFiles.swap(files);
}

}
}
}
