// あ
#pragma once

// 要CoInitialize

#include <memory>

class ShortcutFile
{
public:
	ShortcutFile();
	~ShortcutFile();

	bool SetLinkPath(LPCTSTR path);
	void SetArguments(LPCTSTR args);
	void SetWorkingDirectory(LPCTSTR dir);
	void SetAppId(LPCTSTR appId);
	void SetToastCallbackGUID(GUID guid);

	bool Save(LPCTSTR pathToSave);

	static CString ResolvePath(const CString& linkPath, CString* description = nullptr);
	// 特別なフォルダパス生成
	static void MakeSpecialFolderPath(CString& out, int type, LPCTSTR linkName);

protected:
	CString mLinkPath;
	CString mArguments;
	CString mDir;
	CString mAppId;
	std::unique_ptr<GUID> mToastCallbackGuid;

};
