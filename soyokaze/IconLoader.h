#pragma once

#include <memory>

class IconLoader
{
public:
	static IconLoader* Get();

	HICON LoadIconFromPath(const CString& path);
	// ファイル拡張子に関連付けられたアイコンを取得
	HICON LoadExtensionIcon(const CString& fileExt);
	// 画像ファイルからアイコンを生成
	HICON LoadIconFromImage(const CString& path);
	// ウインドウハンドルからアプリアイコンを取得
	HICON LoadIconFromHwnd(HWND hwnd);

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
	HICON LoadWindowIcon();
	HICON LoadRegisterWindowIcon();
	HICON LoadGroupIcon();
	HICON LoadPromptIcon();

	HICON LoadIconFromStream(const std::vector<uint8_t>& strm);
	static bool GetStreamFromPath(const CString& path, std::vector<uint8_t>& strm);

private:
	IconLoader();
	~IconLoader();

	struct PImpl;
	std::unique_ptr<PImpl> in;
};

