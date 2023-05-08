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

protected:
	CString mLinkPath;
	CString mArguments;
	CString mDir;

};
