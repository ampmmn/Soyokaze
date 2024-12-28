#include "pch.h"
#include "Path.h"
#include "utility/AppProfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


Path::Path() : mPath(MAX_PATH_NTFS)
{
}

Path::Path(LPCTSTR initPath) : mPath(MAX_PATH_NTFS)
{
	_tcsncpy_s(&mPath.front(), (DWORD)mPath.size(), initPath, _TRUNCATE);
}

Path::Path(MODULEFILEPATH_TAG tag, LPCTSTR extraPath) : mPath(MAX_PATH_NTFS)
{
	GetModuleFileName(nullptr, &mPath.front(), (DWORD)mPath.size());
	if (tag == MODULEFILEDIR) {
		RemoveFileSpec();
	}
	if (extraPath != nullptr) {
		Append(extraPath);
	}
}

Path::Path(APPPROFILE_TAG, LPCTSTR extraPath) : mPath(MAX_PATH_NTFS)
{
	CAppProfile::GetDirPath(data(), (DWORD)size());
	if (extraPath != nullptr) {
		Append(extraPath);
	}
}

Path::~Path()
{
}

bool Path::IsEmptyPath() const
{
	return mPath.empty() || mPath[0] == _T('\0');
}

Path::operator LPCTSTR() const
{
	return cdata();
}

Path::operator LPTSTR()
{
	return data();
}

size_t Path::size() const
{
	return mPath.size();
}

bool Path::Append(LPCTSTR path)
{
	return PathAppend(data(), path) != FALSE;
}

bool Path::AddExtension(LPCTSTR ext)
{
	return PathAddExtension(data(), ext) != FALSE;
}

LPCTSTR Path::FindFileName() const
{
	return PathFindFileName(cdata());
}

LPCTSTR Path::FindExtension() const
{
	return PathFindExtension(cdata());
}

bool Path::RemoveFileSpec()
{
	return PathRemoveFileSpec(data()) != FALSE;
}

void Path::RemoveExtension()
{
	PathRemoveExtension(data());
}

bool Path::RenameExtension(LPCTSTR ext)
{
	return PathRenameExtension(data(), ext) != false;
}

bool Path::FileExists() const
{
	return PathFileExists(cdata()) != FALSE;
}

bool Path::IsDirectory() const
{
	return PathIsDirectory(cdata()) != FALSE;
}

bool Path::IsURL() const
{
	return PathIsURL(cdata()) != FALSE;
}

bool Path::IsRelative() const
{
	return PathIsRelative(cdata()) != FALSE;
}

HRESULT Path::CreateFromUrl(LPCTSTR url, DWORD flags)
{
	DWORD len = (DWORD)size();
	return PathCreateFromUrl(url, data(), &len, flags);
}

// 実際のパスの長さにbufferを切り詰める
void Path::Shrink()
{
	size_t actualLen = _tcslen(cdata());
	mPath.resize(actualLen + 1);
	mPath.shrink_to_fit();
}

LPCTSTR Path::cdata() const
{
	return &mPath.front();
}
LPTSTR Path::data()
{
	return &mPath.front();
}


