#pragma once

#include <memory>

class IconLoader
{
public:
	static IconLoader* Get();

	HICON LoadIconFromPath(const CString& path);
	HICON LoadExtensionIcon(const CString& fileExt);
	HICON LoadIconFromPNG(const CString& path);

	void RegisterIcon(const CString& path, HICON icon);

	HICON GetDefaultIcon(const CString& path);

	HICON GetShell32Icon(int index);
	HICON GetImageResIcon(int index);

	HICON LoadFolderIcon();
	HICON LoadWebIcon();
	HICON LoadNewIcon();
	HICON LoadSettingIcon();
	HICON LoadExitIcon();
	HICON LoadEditIcon();
	HICON LoadKeywordManagerIcon();
	HICON LoadDefaultIcon();
	HICON LoadUserDirIcon();
	HICON LoadMainDirIcon();
	HICON LoadVersionIcon();
	HICON LoadTasktrayIcon();
	HICON LoadUnknownIcon();
	HICON LoadReloadIcon();
	HICON LoadRegisterWindowIcon();
	HICON LoadGroupIcon();
	HICON LoadPromptIcon();


private:
	IconLoader();
	~IconLoader();

	struct PImpl;
	std::unique_ptr<PImpl> in;
};

