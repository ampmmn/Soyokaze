#include "pch.h"
#include "framework.h"
#include "IconLoader.h"
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

using LocalPathResolver = soyokaze::utility::LocalPathResolver;

struct IconLoader::PImpl
{
	PImpl() : 
		mEditIcon(nullptr),
		mKeywordManagerIcon(nullptr)
	{
		const LPCTSTR SYSTEMROOT = _T("SystemRoot");

		size_t reqLen = 0;
		_tgetenv_s(&reqLen, mImgResDll, MAX_PATH_NTFS, SYSTEMROOT);
		PathAppend(mImgResDll, _T("System32"));
		PathAppend(mImgResDll, _T("imageres.dll"));

		_tgetenv_s(&reqLen, mShell32Dll, MAX_PATH_NTFS, SYSTEMROOT);
		PathAppend(mShell32Dll, _T("System32"));
		PathAppend(mShell32Dll, _T("shell32.dll"));

	}

	int GetImageResIconCount()
	{
		return (int)ExtractIconEx(mImgResDll, 0, nullptr, nullptr, 0);
	}

	HICON GetImageResIcon(int index)
	{
		HICON icon[1];
		UINT n = ExtractIconEx(mImgResDll, index, icon, NULL, 1);
		return (n == 1) ? icon[0]: NULL;
	}

	HICON GetShell32Icon(int index)
	{
		HICON icon[1];
		UINT n = ExtractIconEx(mShell32Dll, index, icon, NULL, 1);
		return (n == 1) ? icon[0]: NULL;
	}


	TCHAR mImgResDll[MAX_PATH_NTFS];
	TCHAR mShell32Dll[MAX_PATH_NTFS];
	std::map<int, HICON> mShell32IconCache;
	std::map<int, HICON> mImageResIconCache;
	std::map<CString, HICON> mDefaultIconCache;
	std::map<CString, HICON> mFileExtIconCache;
	std::map<CString, HICON> mAppIconMap;
	HICON mEditIcon;
	HICON mKeywordManagerIcon;

	LocalPathResolver mResolver;
};

IconLoader::IconLoader() : in(std::make_unique<PImpl>())
{
}

IconLoader::~IconLoader()
{
	for (auto& elem : in->mShell32IconCache) {
		if (elem.second) {
			DestroyIcon(elem.second);
		}
	}
	for (auto& elem : in->mImageResIconCache) {
		if (elem.second) {
			DestroyIcon(elem.second);
		}
	}
	for (auto& elem : in->mDefaultIconCache) {
		if (elem.second) {
			DestroyIcon(elem.second);
		}
	}
	for (auto& elem : in->mAppIconMap) {
		if (elem.second) {
			DestroyIcon(elem.second);
		}
	}
	// mFileExtIconCacheに格納されているアイコンハンドルはmDefaultIconCacheにも含まれるのでDestroyIconは不要。
	//for (auto& elem : in->mFileExtIconCache) {
	//	if (elem.second) {
	//		DestroyIcon(elem.second);
	//	}
	//}
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
	HIMAGELIST hImgList =
		(HIMAGELIST)::SHGetFileInfo(fullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	HICON hIcon = sfi.hIcon;

	RegisterIcon(fullPath, hIcon);

	return hIcon;
}

HICON IconLoader::LoadExtensionIcon(const CString& fileExt)
{
	auto it = in->mFileExtIconCache.find(fileExt);
	if (it != in->mFileExtIconCache.end()) {
		return it->second;
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
		CString iconPath;
		if (HKCR.GetValue(subKey, _T(""), iconPath) == false) {
			return nullptr;
		}

		// 次回以降は再利用
		HICON icon = GetDefaultIcon(iconPath);
		in->mFileExtIconCache[fileExt] = icon;
		return icon;
	}

	return nullptr;
}

static HICON LoadIconForID1(LPCTSTR dllPath)
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

HICON IconLoader::GetDefaultIcon(const CString& path)
{
	auto it = in->mDefaultIconCache.find(path);
	if (it != in->mDefaultIconCache.end()) {
		return it->second;
	}

	int n = 0;
	CString modulePath = path.Tokenize(_T(","), n);
	if (modulePath.IsEmpty()) {
		return LoadUnknownIcon();
	}
	CString indexStr =  path.Tokenize(_T(","), n);

	int index;
	if (_stscanf_s(indexStr, _T("%d"), &index) != 1) {
		index = 0;
	}

	HICON icon[1] = {};

	// UWPの場合、アイコンが.icoではなく.PNGなので
	// PNGからアイコンに変換する
	static CString extPNG(_T(".png"));
	if (index == 0 && extPNG == PathFindExtension(modulePath)) {
		icon[0] = LoadIconFromImage(modulePath);
	}
	else if (index == -1) {
		// -1のときアイコン総数が返ってきてそうなので別系統の処理をする
		icon[0] = LoadIconForID1(modulePath);
	}
	else {
		ExtractIconEx(modulePath, index, icon, nullptr, 1);
	}

	if (icon[0] == 0)  {
		return LoadUnknownIcon();
	}

	in->mDefaultIconCache[path] = icon[0];

	return icon[0];
}



HICON IconLoader::GetShell32Icon(int index)
{
	auto it = in->mShell32IconCache.find(index);
	if (it != in->mShell32IconCache.end()) {
		return it->second;
	}

	HICON h =in->GetShell32Icon(index);
	if (h == NULL) {
		return nullptr;
	}

	in->mShell32IconCache[index] = h;
	return h;
}

HICON IconLoader::GetImageResIcon(int index)
{
	auto it = in->mImageResIconCache.find(index);
	if (it != in->mImageResIconCache.end()) {
		return it->second;
	}

	HICON h =in->GetImageResIcon(index);
	if (h == NULL) {
		return nullptr;
	}

	in->mImageResIconCache[index] = h;
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
	return GetImageResIcon(-1024);
}

HICON IconLoader::LoadSettingIcon()
{
	return GetImageResIcon(-114);
}

HICON IconLoader::LoadExitIcon()
{
	int n = in->GetImageResIconCount();
	return GetImageResIcon(-5102);
}

static int GetPitch(int w, int bpp)
{
	return (((w * bpp) + 31) / 32) * 4;
}

HICON IconLoader::LoadIconFromImage(const CString& path)
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
		DestroyIcon(it->second);
		in->mAppIconMap.erase(it);
	}
	in->mAppIconMap[appId] = icon;
}

HICON IconLoader::LoadEditIcon()
{
	// ToDo: 実装
	return in->mEditIcon;
}

HICON IconLoader::LoadKeywordManagerIcon()
{
	// ToDo: 実装
	return in->mKeywordManagerIcon;
}

HICON IconLoader::LoadDefaultIcon()
{
	return AfxGetApp()->LoadIcon(IDI_ICON2);
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
	return AfxGetApp()->LoadIcon(IDI_ICON2);
}

HICON IconLoader::LoadTasktrayIcon()
{
	return AfxGetApp()->LoadIcon(IDI_ICON2);
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
		return it->second;
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

	HICON hIcon = LoadIconFromImage(userDataPath);
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


