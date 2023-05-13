#pragma once

// 要CoInitialize

class ShortcutFile
{
public:
	ShortcutFile();
	~ShortcutFile();

	bool SetLinkPath(LPCTSTR path);
	void SetArguments(LPCTSTR args);
	void SetWorkingDirectory(LPCTSTR dir);

	bool Save(LPCTSTR pathToSave);

	static CString ResolvePath(const CString& linkPath);

protected:
	CString mLinkPath;
	CString mArguments;
	CString mDir;

};
