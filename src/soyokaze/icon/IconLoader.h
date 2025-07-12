#pragma once

#include <memory>

class IconLoader
{
public:
	struct ICONITEM;
public:
	// インスタンス取得
	static IconLoader* Get();

	// ファイルに関連付けられたアイコンを取得(SHGetFileInfo経由の取得)
	HICON LoadIconFromPath(const CString& path);
	// ファイル拡張子に関連付けられたアイコンを取得
	HICON LoadExtensionIcon(const CString& fileExt);
	// ウインドウハンドルからアプリアイコンを取得
	HICON LoadIconFromHwnd(HWND hwnd);
	// ファイルがリソースとして保持するアイコンを取得
	HICON LoadIconResource(const CString& path, int index);
	// イメージファイルからアイコンを取得
	HICON LoadIconFromImageFile(const CString& imageFilePath, bool isShared);

	// 指定したアイコンはIconLoaderが管理しているものか?
	bool HasIcon(HICON icon);

	HICON GetDefaultIcon(const CString& path);

	// shell32.dllからアイコンを取得
	HICON GetShell32Icon(int index);
	// imageres.dllからアイコンを取得
	HICON GetImageResIcon(int index);
	// wmploc.dllからアイコンを取得
	HICON GetWMPlocIcon(int index);
	// mmcndmgr.dllからアイコンを取得
	HICON GetMMCndMgrIcon(int index);
	// mssvp.dllからアイコンを取得
	HICON GetMSSvpIcon(int index);

	// フォルダを示すアイコンを取得する
	HICON LoadFolderIcon();
	// Webを示すアイコンを取得する
	HICON LoadWebIcon();
	// 「新規」を示すアイコンを取得する
	HICON LoadNewIcon();
	// 設定を示すアイコンを取得する
	HICON LoadSettingIcon();
	// 終了を示すアイコンを取得する
	HICON LoadExitIcon();
	// 編集を示すアイコンを取得する
	HICON LoadEditIcon();
	// キーワードマネージャを示すアイコンを取得する
	HICON LoadKeywordManagerIcon();
	// デフォルト(→関連付け不明)を示すアイコンを取得する
	HICON LoadDefaultIcon();
	// ユーザフォルダを示すアイコンを取得する
	HICON LoadUserDirIcon();
	// アプリフォルダを示すアイコンを取得する
	HICON LoadMainDirIcon();
	// バージョン情報を示すアイコンを取得する
	HICON LoadVersionIcon();
	// タスクトレイ用アイコンを取得する
	HICON LoadTasktrayIcon();
	// 「不明」を示すアイコンを取得する
	HICON LoadUnknownIcon();
	// リロードを示すアイコンを取得する
	HICON LoadReloadIcon();
	// ウインドウを示すアイコンを取得する
	HICON LoadWindowIcon();
	// ウインドウ登録を示すアイコンを取得する
	HICON LoadRegisterWindowIcon();
	// グループを示すアイコンを取得する
	HICON LoadGroupIcon();
	// プロンプトを示すアイコンを取得する
	HICON LoadPromptIcon();
	// 音量調整を示すアイコンを取得する
	HICON LoadVolumeIcon(bool isMute);
	// 変換を示すアイコンを取得する
	HICON LoadConvertIcon();
	// 履歴を示すアイコンを取得する
	HICON LoadHistoryIcon();

	// アイコンファイルのデータ列からアイコンを生成する
	HICON LoadIconFromStream(const std::vector<uint8_t>& strm);
	// ファイルパスからデータ列を生成する
	static bool GetStreamFromPath(const CString& path, std::vector<uint8_t>& strm);

#ifndef SOYOKAZE_UNITTEST
private:
#else
public:
#endif
	static bool TryGetStreamFromIconPath(const CString& path, std::vector<uint8_t>& strm);

private:
	IconLoader();
	~IconLoader();

	struct PImpl;
	std::unique_ptr<PImpl> in;
};

