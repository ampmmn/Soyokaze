#pragma once

#include <TCHAR.h>
#include <shlwapi.h>
#include <vector>


class Path
{
	class QueryData;
	class QueryDataHost;

public:
	enum MODULEFILEPATH_TAG {
	 	MODULEFILEPATH,
	 	MODULEFILEDIR,
 	};
	enum APPPROFILE_TAG {
		APPDIR,
		APPDIRPERMACHINE,
	};
	enum SYSTEMDIRECTORY_TAG {
		SYSTEMDIR,
		APPDATA,
		LOCALAPPDATA,
		USERPROFILE,
	};

public:
	Path();
	Path(LPCTSTR initPath);
	Path(MODULEFILEPATH_TAG tag, LPCTSTR extraPath = nullptr);
	Path(APPPROFILE_TAG tag, LPCTSTR extraPath = nullptr);
	Path(SYSTEMDIRECTORY_TAG tag, LPCTSTR extraPath = nullptr);
	~Path();

	Path& operator = (const CString& path);

	bool FileUriToLocalPath(const String& uri);
	HRESULT CreateFromUrl(LPCTSTR url, DWORD flags = 0);

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

	// 実際のパスの長さにbufferを切り詰める
	void Shrink();

	static bool FileExists(LPCWSTR path);
	static bool FileExists(LPCSTR path);
	static bool IsDirectory(LPCTSTR path);

	// 指定したホストに対する接続が確立しているかを確認する
	static bool IsHostConnected(const CString& targetHost);

protected:
	LPCTSTR cdata() const;
	LPTSTR data();

protected:
	std::vector<TCHAR> mPath;
};

