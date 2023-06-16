#include "pch.h"
#include "LocalPathResolver.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace soyokaze {
namespace utility {


struct LocalPathResolver::PImpl
{
	std::vector<CString> targetDirs;
};

LocalPathResolver::LocalPathResolver() : in(new PImpl)
{
	LPCTSTR PATH = _T("PATH");

	size_t reqLen = 0;
	if (_tgetenv_s(&reqLen, NULL, 0, PATH) != 0 || reqLen == 0) {
		return;
	}
	
	CString val;
	TCHAR* p = val.GetBuffer((int)reqLen);
	_tgetenv_s(&reqLen, p, reqLen, PATH);
	val.ReleaseBuffer();

	int n = 0;
	CString item = val.Tokenize(_T(";"), n);
	while(item.IsEmpty() == FALSE) {

		// UNCや相対パスは許可しない
		if (PathIsUNC(item)) {
			continue;
		}
		if (PathIsRelative(item)) {
			continue;
		}

		if (PathIsDirectory(item)) {
			in->targetDirs.push_back(item);
		}

		item = val.Tokenize(_T(";"), n);
	}
}

LocalPathResolver::LocalPathResolver(const LocalPathResolver& rhs) 
	: in(new PImpl)
{
	in->targetDirs = rhs.in->targetDirs;
}

LocalPathResolver::~LocalPathResolver()
{
}

LocalPathResolver& LocalPathResolver::operator = (
	const LocalPathResolver& rhs
)
{
	if (this != &rhs) {
		in->targetDirs = rhs.in->targetDirs;
	}
	return *this;
}

bool LocalPathResolver::AddPath(LPCTSTR path)
{
		// UNCや相対パスは許可しない
	if (PathIsUNC(path)) {
		return false;
	}
	if (PathIsRelative(path)) {
		return false;
	}

	in->targetDirs.push_back(path);
	return true;
}

bool LocalPathResolver::Resolve(CString& path)
{
	return Resolve(path, path);
}

bool LocalPathResolver::Resolve(
	const CString& path,
 	CString& resolvedPath
)
{
	if (path.IsEmpty()) {
		return false;
	}

	if (PathIsRelative(path) == FALSE) {
		// 相対パスでなければ処理対象外
		resolvedPath = path;
		return true;
	}

	bool isSupplyExeExt = false;
	LPCTSTR ext = PathFindExtension(path);
	if (_tcslen(ext) == 0) {
		isSupplyExeExt = true;
	}

	TCHAR fullPath[MAX_PATH_NTFS];

	for (const auto& dir : in->targetDirs) {
		_tcscpy_s(fullPath, dir);
		PathAppend(fullPath, path);
		if (isSupplyExeExt) {
			PathAddExtension(fullPath, _T(".exe"));
		}

		if (PathFileExists(fullPath) == FALSE) {
			continue;
		}

		// 解決した
		resolvedPath = fullPath;
		return true;
	}

	// 解決できなかった
	return false;
}


}
}

