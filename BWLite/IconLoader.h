#pragma once

class IconLoader
{
public:
	static IconLoader* Get();

	HICON LoadIconFromPath(const CString& path);

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

private:
	IconLoader();
	~IconLoader();

	struct PImpl;
	PImpl* in;
};

