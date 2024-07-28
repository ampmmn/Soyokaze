#pragma once

#include <TCHAR.h>
#include <shlwapi.h>
#include <vector>


class Path
{
public:
	enum MODULEFILEPATH_TAG {
	 	MODULEFILEPATH,
	 	MODULEFILEDIR,
 	};
	enum APPPROFILE_TAG {
		APPDIR,
	};

public:
	Path();
	Path(LPCTSTR initPath);
	Path(MODULEFILEPATH_TAG tag, LPCTSTR extraPath = nullptr);
	Path(APPPROFILE_TAG tag, LPCTSTR extraPath = nullptr);
	~Path();

	bool IsEmptyPath() const;
	operator LPCTSTR() const;
	operator LPTSTR();

	size_t size() const;
	bool Append(LPCTSTR path);
	bool AddExtension(LPCTSTR ext);

	LPCTSTR FindFileName() const;
	LPCTSTR FindExtension() const;

	bool RemoveFileSpec();
	void RemoveExtension();
	bool RenameExtension(LPCTSTR ext);
	bool FileExists() const;
	bool IsDirectory() const;
	bool IsURL() const;

	bool IsRelative() const;
	HRESULT CreateFromUrl(LPCTSTR url, DWORD flags = 0);

	// 実際のパスの長さにbufferを切り詰める
	void Shrink();

protected:
	LPCTSTR cdata() const;
	LPTSTR data();

protected:
	std::vector<TCHAR> mPath;
};

