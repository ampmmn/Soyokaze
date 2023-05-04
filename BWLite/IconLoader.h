#pragma once

class IconLoader
{
public:
	static IconLoader* Get();

	HICON LoadFolderIcon();
	HICON LoadWebIcon();
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

