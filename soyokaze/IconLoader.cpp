#include "pch.h"
#include "framework.h"
#include "IconLoader.h"
#include "utility/LocalPathResolver.h"
#include "utility/RegistryKey.h"
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

	if (PathIsDirectory(fullPath)) {
		return LoadFolderIcon();
	}

	SHFILEINFO sfi = {};
	HIMAGELIST hImgList =
		(HIMAGELIST)::SHGetFileInfo(fullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
	HICON hIcon = sfi.hIcon;
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
		icon[0] = LoadIconFromPNG(modulePath);
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
	return GetImageResIcon(3);
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
	// ToDo: 実装
	return AfxGetApp()->LoadIcon(IDI_ICON2);
}

HICON IconLoader::LoadSettingIcon()
{
	return GetImageResIcon(109);
}

HICON IconLoader::LoadExitIcon()
{
	int n = in->GetImageResIconCount();
	if (n == 334) {
		// 暫定: imageres.dllに含まれるiconの数でインデックスを判断する(こっちはWin10)
		return GetImageResIcon(235);
	}
	else {
		// Win11
		return GetImageResIcon(236);
	}
}

static int GetPitch(int w, int bpp)
{
	return (((w * bpp) + 31) / 32) * 4;
}

HICON IconLoader::LoadIconFromPNG(const CString& path)
{
	ATL::CImage image;
	HRESULT hr = image.Load(path);
	if (FAILED(hr)) {
		return nullptr;
	}

	CSize size(image.GetWidth(), image.GetHeight());

	if (image.GetBPP() != 32) {
		// 透過情報を持たないものは非対応
		return nullptr;
	}

	ATL::CImage imgMask;
	imgMask.Create(size.cx, size.cy, 1);

	ICONINFO ii;
	ii.fIcon = TRUE;
	ii.xHotspot = 0;
	ii.yHotspot = 0;
	ii.hbmMask = (HBITMAP)imgMask;
	ii.hbmColor = (HBITMAP)image;


	HICON icon = CreateIconIndirect(&ii);

	return icon;
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
	return GetImageResIcon(157);
}

HICON IconLoader::LoadMainDirIcon()
{
	return GetImageResIcon(157);
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
	return GetImageResIcon(2);
}

HICON IconLoader::LoadReloadIcon()
{
	int n = in->GetImageResIconCount();
	if (n == 334) {
		// 暫定: imageres.dllに含まれるiconの数でインデックスを判断する(こっちはWin10)
		return GetImageResIcon(228);
	}
	else {
		// Win11
		return GetImageResIcon(229);
	}

}

HICON IconLoader::LoadRegisterWindowIcon()
{
	return GetImageResIcon(19);
}

HICON IconLoader::LoadGroupIcon()
{
	return GetShell32Icon(250);
}

HICON IconLoader::LoadPromptIcon()
{
	int n = in->GetImageResIconCount();
	if (n == 334) {
		// 暫定: imageres.dllに含まれるiconの数でインデックスを判断する(こっちはWin10)
		return GetImageResIcon(311);
	}
	else {
		// Win11
		return GetImageResIcon(312);
	}
}

