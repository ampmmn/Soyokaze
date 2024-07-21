#include "pch.h"
#include "framework.h"
#include "IconLoader.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "utility/LocalPathResolver.h"
#include "utility/RegistryKey.h"
#include "utility/AppProfile.h"
#include "utility/SHA1.h"
#include "utility/ProcessPath.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <map>
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// キャッシュクリアを実行する回数
constexpr int CLEARCACHE_INTERVAL = 5 * 60;  // 5分

// 前回使用時から経過した時間がこの時間を過ぎていたらキャッシュをクリアする
constexpr int CACHE_KEEP_IN_MS = 5 * 60 * 1000;  // 5分


struct IconLoader::ICONITEM
{
	ICONITEM() = default;

	ICONITEM(HICON icon) : mIcon(icon), mLastUsedTime(GetTickCount())
	{
	}
	ICONITEM(const ICONITEM&) = default;

	HICON IconHandle()
	{
		mLastUsedTime = GetTickCount();
		return mIcon;
	}

	bool IsTimeout(DWORD now) const
 	{
		DWORD elapsed = now - mLastUsedTime;
		return elapsed >= CACHE_KEEP_IN_MS;
	}

	void Destroy()
	{
		if (mIcon) {
			DestroyIcon(mIcon);
			mIcon = nullptr;
		}
	}

	HICON mIcon;
	DWORD mLastUsedTime;
};

using ICONITEM = IconLoader::ICONITEM;

using LocalPathResolver = launcherapp::utility::LocalPathResolver;

using IconIndexMap = std::map<int, ICONITEM>;

struct IconLoader::PImpl : public LauncherWindowEventListenerIF
{
	PImpl() : 
		mVolumeIcon(nullptr),
		mVolumeMuteIcon(nullptr)
	{
		const LPCTSTR SYSTEMROOT = _T("SystemRoot");

		size_t reqLen = 0;
		_tgetenv_s(&reqLen, mImgResDll, MAX_PATH_NTFS, SYSTEMROOT);
		PathAppend(mImgResDll, _T("System32"));
		PathAppend(mImgResDll, _T("imageres.dll"));

		_tgetenv_s(&reqLen, mShell32Dll, MAX_PATH_NTFS, SYSTEMROOT);
		PathAppend(mShell32Dll, _T("System32"));
		PathAppend(mShell32Dll, _T("shell32.dll"));

		_tgetenv_s(&reqLen, mWMPlocDll, MAX_PATH_NTFS, SYSTEMROOT);
		PathAppend(mWMPlocDll, _T("System32"));
		PathAppend(mWMPlocDll, _T("wmploc.dll"));

		LauncherWindowEventDispatcher::Get()->AddListener(this);
	}
	~PImpl()
	{
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);
	}

	HICON GetShell32Icon(int index)
	{
		HICON icon[1];
		UINT n = ExtractIconEx(mShell32Dll, index, icon, NULL, 1);
		return (n == 1) ? icon[0]: NULL;
	}

	IconIndexMap& GetIconIndexMap(const CString& path)
	{
		auto it = mIconIndexCache.find(path);
		if (it != mIconIndexCache.end()) {
			return it->second;
		}
		mIconIndexCache[path] = IconIndexMap();

		return mIconIndexCache[path];
	}

	HICON LoadIconFromImage(const CString& path);
	HICON LoadIconForID1(LPCTSTR dllPath);

	void ClearCache();
	bool GetDefaultIcon(const CString& path, HICON& icon);

	void OnLockScreenOccurred() override
	{
	}

	void OnUnlockScreenOccurred() override
	{
	}

	void OnTimer() override
	{
		mCleanCount--;
		if (mCleanCount < 0) {
			ClearCache();
			mCleanCount = CLEARCACHE_INTERVAL;
		}
	}


	TCHAR mImgResDll[MAX_PATH_NTFS];
	TCHAR mShell32Dll[MAX_PATH_NTFS];
	TCHAR mWMPlocDll[MAX_PATH_NTFS];

	// ファイルがリソースとして保持するアイコンを管理するためのmap
	std::map<CString, IconIndexMap> mIconIndexCache;
	// ファイルパスに対するアイコン
	std::map<CString, ICONITEM> mDefaultIconCache;
	// ファイル拡張子に関連付けられたアイコン
	std::map<CString, ICONITEM> mFileExtIconCache;
	// アプリケーションIDに対応するアイコン
	std::map<CString, ICONITEM> mAppIconMap;
	// 音量変更アイコン
	HICON mVolumeIcon;
	// 音量変更(ミュート)のアイコン
	HICON mVolumeMuteIcon;

	LocalPathResolver mResolver;

	int mCleanCount = CLEARCACHE_INTERVAL;
};

void IconLoader::PImpl::ClearCache()
{
	spdlog::debug(_T("ClearCache in."));

	int clearedItemCount = 0;

	auto now = GetTickCount();

	for (auto& item : mIconIndexCache) {
		auto& indexMap = item.second;

		auto it = indexMap.begin();
		while (it != indexMap.end()) {
			auto& iconItem = it->second;
			if (iconItem.IsTimeout(now) == false) {
				++it;
				continue;
			}
			iconItem.Destroy();
			it = indexMap.erase(it);
			clearedItemCount++;
		}
	}

	auto it = mDefaultIconCache.begin();
	while (it != mDefaultIconCache.end()) {
		auto& iconItem = it->second;
		if (iconItem.IsTimeout(now) == false) {
			++it;
			continue;
		}
		iconItem.Destroy();
		it = mDefaultIconCache.erase(it);
		clearedItemCount++;
	}

	it = mDefaultIconCache.begin();
	while (it != mDefaultIconCache.end()) {
		auto& iconItem = it->second;
		if (iconItem.IsTimeout(now) == false) {
			++it;
			continue;
		}
		iconItem.Destroy();
		it = mDefaultIconCache.erase(it);
		clearedItemCount++;
	}

	it = mFileExtIconCache.begin();
	while (it != mFileExtIconCache.end()) {
		auto& iconItem = it->second;
		if (iconItem.IsTimeout(now) == false) {
			++it;
			continue;
		}
		iconItem.Destroy();
		it = mFileExtIconCache.erase(it);
		clearedItemCount++;
	}


	it = mAppIconMap.begin();
	while (it != mAppIconMap.end()) {
		auto& iconItem = it->second;
		if (iconItem.IsTimeout(now) == false) {
			++it;
			continue;
		}
		iconItem.Destroy();
		it = mAppIconMap.erase(it);
		clearedItemCount++;
	}
	spdlog::info(_T("IconLoader ClearCache {} icons cleared."), clearedItemCount);
}

bool IconLoader::PImpl::GetDefaultIcon(const CString& path, HICON& icon)
{
	int n = 0;
	CString modulePath = path.Tokenize(_T(","), n);
	if (modulePath.IsEmpty()) {
		return false;
	}
	CString indexStr =  path.Tokenize(_T(","), n);

	int index;
	if (_stscanf_s(indexStr, _T("%d"), &index) != 1) {
		index = 0;
	}

	// UWPの場合、アイコンが.icoではなく.PNGなので
	// PNGからアイコンに変換する
	static CString extPNG(_T(".png"));
	if (index == 0 && extPNG == PathFindExtension(modulePath)) {
		icon = LoadIconFromImage(modulePath);
	}
	else if (index == -1) {
		// -1のときアイコン総数が返ってきてそうなので別系統の処理をする
		icon = LoadIconForID1(modulePath);
	}
	else {
		ExtractIconEx(modulePath, index, &icon, nullptr, 1);
	}

	if (icon == nullptr)  {
		return false;
	}

	return true;
}


HICON IconLoader::PImpl::LoadIconFromImage(const CString& path)
{
	// 画像ファイルをロードする
	ATL::CImage image;
	HRESULT hr = image.Load(path);
	if (FAILED(hr)) {
		return nullptr;
	}

	CSize size(image.GetWidth(), image.GetHeight());

	ATL::CImage imageResize;
	
	HBITMAP imgHandle = (HBITMAP)image;

	// サイズが大きすぎる場合はリサイズする
	if (size.cx >= 64 || size.cy >= 64) {

		// 縦横比を保持したまま縮小サイズを計算する
		int cx = size.cx > size.cy ? 64 : (int)(64 * (size.cx / (double)size.cy));
		int cy = size.cx < size.cy ? 64 : (int)(64 * (size.cy / (double)size.cx));
		if (cx < 0) { cx = 1; }
		if (cy < 0) { cy = 1; }

		// 縮小画像の生成
		imageResize.Create(cx, cy, image.GetBPP());

		image.Draw(imageResize.GetDC(), 0, 0, cx, cy);
		imageResize.ReleaseDC();

		imgHandle = (HBITMAP)imageResize;
		size = CSize(cx, cy);
	}

	// マスク画像の初期化
	ATL::CImage imgMask;
	imgMask.Create(size.cx, size.cy, 1);
	BYTE* head = (BYTE*)imgMask.GetBits();
	int pitchAbs = abs(imgMask.GetPitch());
	for (int y = 0; y < size.cy; ++y) {
		BYTE* p = head + (imgMask.GetPitch() * y);
		memset(p, 0xff, pitchAbs);
	}

	// アイコンの作成
	ICONINFO ii;
	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmMask = (HBITMAP)imgMask;
	ii.hbmColor = imgHandle;


	HICON icon = CreateIconIndirect(&ii);

	return icon;
}

HICON IconLoader::PImpl::LoadIconForID1(LPCTSTR dllPath)
{
	HMODULE dll = LoadLibraryEx(dllPath, nullptr, LOAD_LIBRARY_AS_DATAFILE | LOAD_WITH_ALTERED_SEARCH_PATH);
	if (dll == nullptr) {
		return nullptr;
	}

	HRSRC hRes2 = FindResource(dll, MAKEINTRESOURCE(1), RT_ICON);
	HGLOBAL hMem2 = LoadResource(dll, hRes2);
	void* lpv2 = LockResource(hMem2);
	HICON icon = CreateIconFromResource((PBYTE) lpv2, SizeofResource(dll, hRes2), TRUE, 0x00030000);

	FreeLibrary(dll);
	return icon;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



IconLoader::IconLoader() : in(std::make_unique<PImpl>())
{
}

IconLoader::~IconLoader()
{
	for (auto& elem : in->mIconIndexCache) {
		for (auto& elem2 : elem.second) {
			auto& item = elem2.second;
			item.Destroy();
		}
	}
	for (auto& elem : in->mDefaultIconCache) {
		auto& item = elem.second;
		item.Destroy();
	}
	for (auto& elem : in->mAppIconMap) {
		auto& item = elem.second;
		item.Destroy();
	}
	for (auto& elem : in->mFileExtIconCache) {
		auto& item = elem.second;
		item.Destroy();
	}

	if (in->mVolumeIcon) {
		DestroyIcon(in->mVolumeIcon);
	}
	if (in->mVolumeMuteIcon) {
		DestroyIcon(in->mVolumeMuteIcon);
	}
}

IconLoader* IconLoader::Get()
{
	static IconLoader instance;
	return &instance;
}

HICON IconLoader::LoadIconFromPath(const CString& path)
{
	if (path.Find(_T("http")) == 0) {
		// URLの場合は固定のWebアイコンにする
		return LoadWebIcon();
	}

	// 絶対パス指定でファイルが見つからなかった場合は不明アイコン
	if (PathIsRelative(path) == FALSE) {
		if (PathFileExists(path) == FALSE) {
			return LoadUnknownIcon();
		}
	}

	// 相対パス指定の場合はパス解決する
	CString fullPath(path);
	if (PathIsRelative(path)) {
		if (in->mResolver.Resolve(path, fullPath) == false) {
			return LoadUnknownIcon();
		}
	}

	// パスがフォルダだった場合はフォルダアイコン
	if (PathIsDirectory(fullPath)) {
		return LoadFolderIcon();
	}

	SHFILEINFO sfi = {};
	::SHGetFileInfo(fullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	HICON hIcon = sfi.hIcon;

	RegisterIcon(fullPath, hIcon);

	return hIcon;
}

HICON IconLoader::LoadExtensionIcon(const CString& fileExt)
{
	auto it = in->mFileExtIconCache.find(fileExt);
	if (it != in->mFileExtIconCache.end()) {
		return it->second.IconHandle();
	}

	CString iconPath;
	CString subKey = fileExt;
	subKey += _T("\\OpenWithProgIDs");
	RegistryKey HKCR(HKEY_CLASSES_ROOT);
	std::vector<CString> valueNames;
	if (HKCR.EnumValueNames(subKey, valueNames) == false) {
		return nullptr;
	}

	for (auto& valueName : valueNames) {
		if (valueName.IsEmpty()) {
			continue;
		}

		subKey = valueName + _T("\\DefaultIcon");
		if (HKCR.GetValue(subKey, _T(""), iconPath) == false) {
			return nullptr;
		}

		// 次回以降は再利用
		HICON icon = nullptr;
		if (in->GetDefaultIcon(iconPath, icon) == false) {
			continue;
		}
		in->mFileExtIconCache[fileExt] = ICONITEM(icon);
		return icon;
	}

	return nullptr;
}

HICON IconLoader::GetDefaultIcon(const CString& path)
{
	auto it = in->mDefaultIconCache.find(path);
	if (it != in->mDefaultIconCache.end()) {
		auto& item = it->second;
		return item.IconHandle();
	}
	HICON icon = nullptr;
	if (in->GetDefaultIcon(path, icon) == false)  {
		return LoadUnknownIcon();
	}

	in->mDefaultIconCache[path] = ICONITEM(icon);

	return icon;
}



HICON IconLoader::GetShell32Icon(int index)
{
	return LoadIconResource(in->mShell32Dll, index);
}

HICON IconLoader::GetImageResIcon(int index)
{
	return LoadIconResource(in->mImgResDll, index);
}

HICON IconLoader::GetWMPlocIcon(int index)
{
	return LoadIconResource(in->mWMPlocDll, index);
}

// ファイルがリソースとして保持するアイコンを取得
HICON IconLoader::LoadIconResource(const CString& path, int index)
{
	IconIndexMap& idxMap = in->GetIconIndexMap(path);

	auto it = idxMap.find(index);
	if (it != idxMap.end()) {
		return it->second.IconHandle();
	}

	HICON icon[1];
	UINT n = ExtractIconEx(path, index, icon, nullptr, 1);
	HICON h = (n == 1) ? icon[0]: nullptr;
	if (h == nullptr) {
		return nullptr;
	}

	idxMap[index] = ICONITEM(h);
	return h;
}


HICON IconLoader::LoadFolderIcon()
{
	return GetImageResIcon(-3);
}

static CString GetHttpsIconPathFromRegistry()
{
	CString progName;
	RegistryKey HKCU(HKEY_CURRENT_USER);
	if (HKCU.GetValue(_T("SOFTWARE\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\https\\UserChoice"), _T("ProgId"), progName) == false) {
		return _T("");
	}

	CString iconPath;
	CString subKey = progName;
	subKey += _T("\\DefaultIcon");
	RegistryKey HKCR(HKEY_CLASSES_ROOT);
	if (HKCR.GetValue(subKey, _T(""), iconPath) == false) {
		return _T("");
	}
	return iconPath;
}

HICON IconLoader::LoadWebIcon()
{
	return GetDefaultIcon(GetHttpsIconPathFromRegistry());
}

HICON IconLoader::LoadNewIcon()
{
	return GetWMPlocIcon(-474);
}

HICON IconLoader::LoadSettingIcon()
{
	return GetImageResIcon(-114);
}

HICON IconLoader::LoadExitIcon()
{
	return GetImageResIcon(-5102);
}

static int GetPitch(int w, int bpp)
{
	return (((w * bpp) + 31) / 32) * 4;
}

// ウインドウハンドルからアプリアイコンを取得
HICON IconLoader::LoadIconFromHwnd(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		return LoadWindowIcon();
	}

	ProcessPath processPath(hwnd);

	// 自プロセスのウインドウなら何もしない
	if (GetCurrentProcessId() == processPath.GetProcessId()) {
		return LoadWindowIcon();
	}

	try {
		CString path = processPath.GetProcessPath();
		return GetDefaultIcon(path);
	}
	catch (ProcessPath::Exception&) {
		return LoadWindowIcon();
	}
}

void IconLoader::RegisterIcon(const CString& appId, HICON icon)
{
	auto it = in->mAppIconMap.find(appId);
	if (it != in->mAppIconMap.end()) {
		it->second.Destroy();
		in->mAppIconMap.erase(it);
	}
	in->mAppIconMap[appId] = ICONITEM(icon);
}

HICON IconLoader::LoadEditIcon()
{
	return GetShell32Icon(-16826);
}

HICON IconLoader::LoadKeywordManagerIcon()
{
	return GetShell32Icon(-16826);
}

HICON IconLoader::LoadDefaultIcon()
{
	return AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

HICON IconLoader::LoadUserDirIcon()
{
	return GetImageResIcon(-166);
}

HICON IconLoader::LoadMainDirIcon()
{
	return GetImageResIcon(-166);
}

HICON IconLoader::LoadVersionIcon()
{
	return AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

HICON IconLoader::LoadTasktrayIcon()
{
	return AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

HICON IconLoader::LoadUnknownIcon()
{
	return GetImageResIcon(-2);
}

HICON IconLoader::LoadReloadIcon()
{
	return GetImageResIcon(-1401);
}

HICON IconLoader::LoadWindowIcon()
{
	return GetImageResIcon(-15);
}


HICON IconLoader::LoadRegisterWindowIcon()
{
	return GetImageResIcon(-24);
}

HICON IconLoader::LoadGroupIcon()
{
	return GetShell32Icon(-133);
}

HICON IconLoader::LoadPromptIcon()
{
	return GetImageResIcon(-5372);
}

HICON IconLoader::LoadVolumeIcon(bool isMute)
{
	const LPCTSTR SYSTEMROOT = _T("SystemRoot");
	TCHAR path[MAX_PATH_NTFS];
	size_t reqLen = 0;

	_tgetenv_s(&reqLen, path, MAX_PATH_NTFS, SYSTEMROOT);
	PathAppend(path, _T("System32"));
	PathAppend(path, _T("mmsys.cpl"));

	if (isMute) {
		if (in->mVolumeMuteIcon) {
			return in->mVolumeMuteIcon;
		}
		HICON icon[1];
		ExtractIconEx(path, -111, icon, NULL, 1);
		in->mVolumeMuteIcon = icon[0];
		return in->mVolumeMuteIcon;
	}
	else {
		if (in->mVolumeIcon) {
			return in->mVolumeIcon;
		}
		HICON icon[1];
		ExtractIconEx(path, -110, icon, NULL, 1);
		in->mVolumeIcon = icon[0];
		return in->mVolumeIcon;
	}
}

static bool GetTempFilePath(LPTSTR userDataPath, size_t len)
{
	CAppProfile::GetDirPath(userDataPath, len);

	PathAppend(userDataPath, _T("tmp"));
	if (PathIsDirectory(userDataPath) == FALSE) {
		if (CreateDirectory(userDataPath, nullptr) == FALSE) {
			return false;
		}
	}
	PathAppend(userDataPath, _T("icondata.png"));
	return true;
}

/**
 	データ列からアイコンオブジェクトを生成する
 	@return アイコンハンドル
 	@param[in] strm データ列
*/
HICON IconLoader::LoadIconFromStream(
	const std::vector<uint8_t>& strm
)
{
	SHA1 sha;
	sha.Add(strm);
	auto str = sha.Finish();

	auto it = in->mAppIconMap.find(str);
	if (it != in->mAppIconMap.end()) {
		// すでに作成済
		return it->second.IconHandle();
	}

	// アイコンを一時ファイルに書き出す
	TCHAR userDataPath[MAX_PATH_NTFS];
	if (GetTempFilePath(userDataPath, MAX_PATH_NTFS) == false) {
		return nullptr;
	}

	HANDLE h = CreateFile(userDataPath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h == INVALID_HANDLE_VALUE) {
		return nullptr;
	}
	DWORD writtenBytes = 0;
	WriteFile(h, &strm.front(), (DWORD)strm.size(), &writtenBytes, nullptr);
	CloseHandle(h);

	HICON hIcon = in->LoadIconFromImage(userDataPath);
	RegisterIcon(str, hIcon);

	return hIcon;
}

/**
 	Enter description here.
 	@return 
 	@param[in]  path データファイルのパス
 	@param[out] strm データ列
*/
bool IconLoader::GetStreamFromPath(
	const CString& path,
	std::vector<uint8_t>& strm
)
{
	// 画像ファイルをロードする
	ATL::CImage image;
	HRESULT hr = image.Load(path);
	if (FAILED(hr)) {
		return false;
	}

	CSize size(image.GetWidth(), image.GetHeight());

	ATL::CImage imageResize;

	TCHAR tmpFilePath[MAX_PATH_NTFS];
	GetTempFilePath(tmpFilePath, MAX_PATH_NTFS);
	
	// サイズが大きすぎる場合はリサイズする
	// (アイコン表示用の画像なので大きいサイズは想定しない)
	LPCTSTR p = path;
	if (size.cx >= 64 || size.cy >= 64) {

		// 縦横比を保持したまま縮小サイズを計算する
		int cx = size.cx > size.cy ? 64 : (int)(64 * (size.cx / (double)size.cy));
		int cy = size.cx < size.cy ? 64 : (int)(64 * (size.cy / (double)size.cx));
		if (cx < 0) { cx = 1; }
		if (cy < 0) { cy = 1; }

		// 縮小画像の生成
		imageResize.Create(cx, cy, image.GetBPP());

		image.Draw(imageResize.GetDC(), 0, 0, cx, cy);
		imageResize.ReleaseDC();

		imageResize.Save(tmpFilePath);

		p = tmpFilePath;
	}

	HANDLE h = CreateFile(p, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}

	// アイコンファイルなので大きいファイルサイズは想定しない
	DWORD fileSize = GetFileSize(h, nullptr);

	strm.resize(fileSize);

	DWORD readBytes = 0;
	ReadFile(h, &strm.front(), fileSize, &readBytes, nullptr);

	CloseHandle(h);

	return true;
}


