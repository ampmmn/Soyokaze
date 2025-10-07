#include "pch.h"
#include "framework.h"
#include "IconLoader.h"
#include "icon/AppIcon.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "utility/LocalPathResolver.h"
#include "utility/RegistryKey.h"
#include "utility/AppProfile.h"
#include "utility/SHA1.h"
#include "utility/ProcessPath.h"
#include "utility/Path.h"
#include "resource.h"
#include <map>
#include <set>
#include <atlimage.h>
#include <mutex>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::icon;

// キャッシュクリアを実行する回数
constexpr int CLEARCACHE_INTERVAL = 5 * 60;  // 5分

// 前回使用時から経過した時間がこの時間を過ぎていたらキャッシュをクリアする
constexpr int CACHE_KEEP_IN_MS = 5 * 60 * 1000;  // 5分


struct IconLoader::ICONITEM
{
	ICONITEM() = default;

	ICONITEM(HICON icon) : mIcon(icon), mLastUsedTime(GetTickCount64())
	{
	}
	ICONITEM(const ICONITEM&) = default;

	HICON IconHandle()
	{
		mLastUsedTime = GetTickCount64();
		return mIcon;
	}

	bool IsTimeout(uint64_t now) const
 	{
		uint64_t elapsed = now - mLastUsedTime;
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
	uint64_t mLastUsedTime;
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
		_tgetenv_s(&reqLen, mImgResDll, mImgResDll.size(), SYSTEMROOT);
		mImgResDll.Append(_T("System32\\imageres.dll"));
		mImgResDll.Shrink();

		_tgetenv_s(&reqLen, mShell32Dll, mShell32Dll.size(), SYSTEMROOT);
		mShell32Dll.Append(_T("System32\\shell32.dll"));
		mShell32Dll.Shrink();

		_tgetenv_s(&reqLen, mWMPlocDll, mWMPlocDll.size(), SYSTEMROOT);
		mWMPlocDll.Append(_T("System32\\wmploc.dll"));
		mWMPlocDll.Shrink();

		_tgetenv_s(&reqLen, mMMCndMgrDll, mMMCndMgrDll.size(), SYSTEMROOT);
		mMMCndMgrDll.Append(_T("System32\\mmcndmgr.dll"));
		mMMCndMgrDll.Shrink();

		_tgetenv_s(&reqLen, mMSSvpDll, mMSSvpDll.size(), SYSTEMROOT);
		mMSSvpDll.Append(_T("System32\\mssvp.dll"));
		mMSSvpDll.Shrink();

		LauncherWindowEventDispatcher::Get()->AddListener(this);
	}
	~PImpl()
	{
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);
	}

	// アイコンを登録する
	void RegisterIcon(const CString& path, HICON icon);

	// shell32.dllのアイコンをロードする
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

	HICON LoadIconFromImage(LPCTSTR path);
	HICON LoadIconForID1(LPCTSTR dllPath);
	HICON LoadIconFromDefaultValue(const CString& fileExt);
	HICON LoadIconFromOpenWithProgIDs(const CString& fileExt);

	// キャッシュをクリアする
	void ClearCache();
	// 内部の特定mapのキャッシュをクリアする
	int ClearCache(std::map<CString, ICONITEM>& indexMap, uint64_t now);

	HICON GetAppIcon(const CString& key);

	bool GetDefaultIcon(const CString& path, HICON& icon);

	void Register(HICON icon);

	void OnLockScreenOccurred() override
	{
		mIsScreenLocked = true;

	}

	void OnUnlockScreenOccurred() override
	{
		mIsScreenLocked = false;
	}

	void OnTimer() override
	{
		mCleanCount--;
		if (mCleanCount < 0) {
			ClearCache();
			mCleanCount = CLEARCACHE_INTERVAL;
		}
	}
	void OnLauncherActivate() override
	{
	}
	void OnLauncherUnactivate() override
	{
	}


	Path mImgResDll;
	Path mShell32Dll;
	Path mWMPlocDll;
	Path mMMCndMgrDll;
	Path mMSSvpDll;

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

	//
	std::set<HICON> mIconSet;

	LocalPathResolver mResolver;

	int mCleanCount{CLEARCACHE_INTERVAL};

	bool mIsScreenLocked{false};

	std::mutex mAppIconMapMutex;
};

void IconLoader::PImpl::RegisterIcon(const CString& appId, HICON icon)
{
	std::lock_guard<std::mutex> lock(mAppIconMapMutex);

	auto it = mAppIconMap.find(appId);
	if (it != mAppIconMap.end()) {
		it->second.Destroy();
		mAppIconMap.erase(it);
	}
	mAppIconMap[appId] = ICONITEM(icon);
	Register(icon);
}


void IconLoader::PImpl::ClearCache()
{
	if (mIsScreenLocked) {
		return;
	}
	spdlog::debug(_T("ClearCache in."));
	std::lock_guard<std::mutex> lock(mAppIconMapMutex);

	int clearedItemCount = 0;

	auto now = GetTickCount64();

	for (auto& item : mIconIndexCache) {
		auto& indexMap = item.second;

		auto it = indexMap.begin();
		while (it != indexMap.end()) {
			auto& item2 = it->second;
			if (item2.IsTimeout(now) == false) {
				++it;
				continue;
			}

			HICON h = item2.IconHandle();
			auto itSet = mIconSet.find(h);
			if (itSet != mIconSet.end()) {
				mIconSet.erase(itSet);
			}

			item2.Destroy();
			it = indexMap.erase(it);
			clearedItemCount++;
		}
	}

	clearedItemCount += ClearCache(mDefaultIconCache, now);
	clearedItemCount += ClearCache(mFileExtIconCache, now);

	clearedItemCount += ClearCache(mAppIconMap, now);

	if (clearedItemCount > 0) {
		spdlog::info(_T("IconLoader ClearCache {0} icons cleared."), clearedItemCount);
	}
}

int IconLoader::PImpl::ClearCache(std::map<CString, ICONITEM>& indexMap, uint64_t now)
{
	int clearedCount = 0;
	auto it = indexMap.begin();
	while (it != indexMap.end()) {
		auto& item = it->second;
		if (item.IsTimeout(now) == false) {
			++it;
			continue;
		}

		HICON h = item.IconHandle();
		auto itSet = mIconSet.find(h);
		if (itSet != mIconSet.end()) {
			mIconSet.erase(itSet);
		}

		item.Destroy();
		it = indexMap.erase(it);
		clearedCount++;
	}
	return clearedCount;
}

bool IconLoader::PImpl::GetDefaultIcon(const CString& path, HICON& icon)
{
	int n = 0;
	CString modulePath{path.Tokenize(_T(","), n)};
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

// mAppIconMapに登録されたアイコンを取得する
HICON IconLoader::PImpl::GetAppIcon(const CString& key)
{
	std::lock_guard<std::mutex> lock(mAppIconMapMutex);
	auto it = mAppIconMap.find(key);
	if (it != mAppIconMap.end()) {
		auto& item = it->second;
		return item.IconHandle();
	}
	return nullptr;
}

// IconLoaderが管理しているアイコンであることを登録する
void IconLoader::PImpl::Register(HICON icon)
{
	// ここにあるアイコンはこのクラス内のどこかで保持されている
	mIconSet.insert(icon);
}


HICON IconLoader::PImpl::LoadIconFromImage(LPCTSTR path)
{
	// icoとして読み込んでみる
	HICON iconHandle = (HICON)LoadImage(nullptr, path, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	if (iconHandle != nullptr) {
		return iconHandle;
	}

	// icoとして読めなかった場合は、画像ファイルをロードする
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
	if (hRes2 == nullptr) {
		return nullptr;
	}
	HGLOBAL hMem2 = LoadResource(dll, hRes2);
	if (hMem2 == nullptr) {
		return nullptr;
	}
	void* lpv2 = LockResource(hMem2);
	HICON icon = CreateIconFromResource((PBYTE) lpv2, SizeofResource(dll, hRes2), TRUE, 0x00030000);

	FreeLibrary(dll);
	return icon;
}

HICON IconLoader::PImpl::LoadIconFromDefaultValue(const CString& fileExt)
{
	RegistryKey HKCR(HKEY_CLASSES_ROOT);

	CString value;
	if (HKCR.GetValue(fileExt, _T(""), value) == false) {
		return nullptr;
	}

	CString iconPath;
	CString subKey = value + _T("\\DefaultIcon");
	if (HKCR.GetValue(subKey, _T(""), iconPath) == false) {
		return nullptr;
	}

	HICON icon = nullptr;
	if (GetDefaultIcon(iconPath, icon) == false) {
		return nullptr;
	}
	// 次回以降は再利用できるよう保持しておく
	std::lock_guard<std::mutex> lock(mAppIconMapMutex);
	mFileExtIconCache[fileExt] = ICONITEM(icon);
	Register(icon);
	return icon;
}

HICON IconLoader::PImpl::LoadIconFromOpenWithProgIDs(const CString& fileExt)
{
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

		HICON icon = nullptr;
		if (GetDefaultIcon(iconPath, icon) == false) {
			continue;
		}
		// 次回以降は再利用できるよう保持しておく
		std::lock_guard<std::mutex> lock(mAppIconMapMutex);
		mFileExtIconCache[fileExt] = ICONITEM(icon);
		Register(icon);
		return icon;
	}

	return nullptr;
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
	{
		std::lock_guard<std::mutex> lock(in->mAppIconMapMutex);
		for (auto& elem : in->mAppIconMap) {
			auto& item = elem.second;
			item.Destroy();
		}
	}
	for (auto& elem : in->mFileExtIconCache) {
		auto& item = elem.second;
		item.Destroy();
	}

	in->mIconSet.clear();

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
	Path pathObj(path);
	if (pathObj.IsRelative() == false && pathObj.FileExists() == false) {
		return LoadUnknownIcon();
	}

	// 相対パス指定の場合はパス解決する
	CString fullPath(path);
	if (PathIsRelative(path)) {
		if (in->mResolver.Resolve(path, fullPath) == false) {
			return LoadUnknownIcon();
		}
	}

	// パスがフォルダだった場合はフォルダアイコン
	Path fullPathObj(fullPath);
	if (fullPathObj.IsDirectory()) {
		return LoadFolderIcon();
	}

	{
		std::lock_guard<std::mutex> lock(in->mAppIconMapMutex);
		// キャッシュに存在する場合はそれを返す
		auto it = in->mAppIconMap.find(fullPath);
		if (it != in->mAppIconMap.end()) {
			// すでに作成済
			return it->second.IconHandle();
		}
	}

	// SHGetFileInfoは環境により、呼び出しが返るのに時間がかかる(ブロックする)場合がある
	// このためバックグラウンドで実行し、初回はUnknownアイコンを返す
	// 解決後はmAppIconMap経由のキャッシュを返すようにする
	auto waitEventHandle = CreateEvent(nullptr, TRUE, FALSE, nullptr);

	std::thread th([&, waitEventHandle, fullPath]() {
		DWORD tid  = GetCurrentThreadId();
		spdlog::debug(_T("SHGetFileInfo thread start. {0} file:{1}"), tid, (LPCTSTR)fullPath);
		SHFILEINFO sfi = {};
		::SHGetFileInfo(fullPath, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
		HICON hIcon = sfi.hIcon;
		in->RegisterIcon(fullPath, hIcon);

		// 完了した旨をイベント経由で通知する
		SetEvent(waitEventHandle);
		CloseHandle(waitEventHandle);

		spdlog::debug("SHGetFileInfo thread end. {}", tid);
	});
	th.detach();

	// バックグラウンドで実行しているロード処理を最大250msec待つ
	WaitForSingleObject(waitEventHandle, 250);

	auto h = in->GetAppIcon(fullPath);
	if (h != nullptr) {
		return h;
	}
	return LoadUnknownIcon();
}

HICON IconLoader::LoadExtensionIcon(const CString& fileExt)
{
	// キャッシュに存在する場合はそれを返す
	auto it = in->mFileExtIconCache.find(fileExt);
	if (it != in->mFileExtIconCache.end()) {
		return it->second.IconHandle();
	}

	// HKCR/(拡張子)/(既定)からアイコン取得を試みる
	auto icon = in->LoadIconFromDefaultValue(fileExt);
	if (icon) {
		return icon;
	}
	// 取得できない場合はHKCR/(拡張子)/OpenWithProgIDsからアイコン取得を試みる
	return in->LoadIconFromOpenWithProgIDs(fileExt);
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
	in->Register(icon);

	return icon;
}



HICON IconLoader::GetShell32Icon(int index)
{
	return LoadIconResource((LPCTSTR)in->mShell32Dll, index);
}

HICON IconLoader::GetImageResIcon(int index)
{
	return LoadIconResource((LPCTSTR)in->mImgResDll, index);
}

HICON IconLoader::GetWMPlocIcon(int index)
{
	return LoadIconResource((LPCTSTR)in->mWMPlocDll, index);
}

HICON IconLoader::GetMMCndMgrIcon(int index)
{
	return LoadIconResource((LPCTSTR)in->mMMCndMgrDll, index);
}

HICON IconLoader::GetMSSvpIcon(int index)
{
	return LoadIconResource((LPCTSTR)in->mMSSvpDll, index);
}

// ファイルがリソースとして保持するアイコンを取得
HICON IconLoader::LoadIconResource(const CString& path, int index)
{
	std::lock_guard<std::mutex> lock(in->mAppIconMapMutex);

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
	in->Register(h);
	return h;
}

/**
  イメージファイルからアイコンを取得
	isSharedがtrueの場合は所有権をIconLoaderが持つ
	(同じパスに対して再度メソッドを呼び出したときは前回と同じアイコンハンドルを返す)
	isSharedがfalseの場合は呼び出し側がアイコンを破棄する責務を持つ

 	@return アイコンハンドル
 	@param[in] imageFilePath 画像ファイルパス
 	@param[in] isShared      
*/
HICON IconLoader::LoadIconFromImageFile(
	const CString& imageFilePath,
	bool isShared
)
{
	if (isShared) {
		std::lock_guard<std::mutex> lock(in->mAppIconMapMutex);
		// キャッシュに存在する場合はそれを返す
		auto it = in->mAppIconMap.find(imageFilePath);
		if (it != in->mAppIconMap.end()) {
			// すでに作成済
			return it->second.IconHandle();
		}
	}

	HICON iconHandle = in->LoadIconFromImage(imageFilePath);
	if (iconHandle == nullptr) {
		return nullptr;
	}

	if (isShared) {
		// アイコンを登録
		in->RegisterIcon(imageFilePath, iconHandle);
	}
	return iconHandle;
}

// 指定したアイコンはIconLoaderが管理しているものか?
bool IconLoader::HasIcon(HICON icon)
{
	if (icon == LoadDefaultIcon()) {
		return true;
	}
	if (icon == in->mVolumeIcon || icon == in->mVolumeMuteIcon) {
		return true;
	}
	return in->mIconSet.find(icon) != in->mIconSet.end();
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
	return GetShell32Icon(-16826);
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
	return AppIcon::Get()->IconHandle();
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
	return LoadDefaultIcon();
}

HICON IconLoader::LoadTasktrayIcon()
{
	return AppIcon::Get()->DefaultIconHandle();
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
	Path path;
	size_t reqLen = 0;

	_tgetenv_s(&reqLen, path, path.size(), SYSTEMROOT);
	path.Append(_T("System32"));
	path.Append(_T("mmsys.cpl"));

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

HICON IconLoader::LoadConvertIcon()
{
	return GetWMPlocIcon(-29608);
}

HICON IconLoader::LoadHistoryIcon()
{
	return GetImageResIcon(-5353);
}

// コピーを表すアイコンを取得する
HICON IconLoader::LoadCopyIcon()
{
	return GetShell32Icon(-16763);
}

static bool GetTempFilePath(Path& userDataPath)
{
	userDataPath.Append(_T("tmp"));
	if (userDataPath.IsDirectory() == FALSE) {
		if (CreateDirectory(userDataPath, nullptr) == FALSE) {
			return false;
		}
	}
	userDataPath.Append(_T("icondata.png"));
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

	{
		std::lock_guard<std::mutex> lock(in->mAppIconMapMutex);
		auto it = in->mAppIconMap.find(str);
		if (it != in->mAppIconMap.end()) {
			// すでに作成済
			return it->second.IconHandle();
		}
	}

	// アイコンを一時ファイルに書き出す
	Path userDataPath(Path::APPDIR);
	if (GetTempFilePath(userDataPath) == false) {
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
	in->RegisterIcon(str, hIcon);

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
	// アイコンの場合はそれをそのまま使う
	if (TryGetStreamFromIconPath(path, strm)) {
		return true;
	}

	// 画像ファイルをロードする
	ATL::CImage image;
	HRESULT hr = image.Load(path);
	if (FAILED(hr)) {
		return false;
	}

	CSize size(image.GetWidth(), image.GetHeight());

	ATL::CImage imageResize;

	Path tmpFilePath(Path::APPDIR);
	GetTempFilePath(tmpFilePath);
	
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
	if (ReadFile(h, &strm.front(), fileSize, &readBytes, nullptr) == FALSE) {
		SPDLOG_ERROR(_T("Failed to ReadFile! err={:x}"), GetLastError());
	}

	CloseHandle(h);

	return true;
}

// アイコンファイルからよみこむ
bool IconLoader::TryGetStreamFromIconPath(const CString& path, std::vector<uint8_t>& strm)
{
	// アイコンとしてよみこめるか、実際に試してみる
	HICON iconHandle = (HICON)LoadImage(nullptr, (LPCTSTR)path, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	if (iconHandle == nullptr) {
		// アイコンファイルではない
		return false;
	}
	DestroyIcon(iconHandle);

	// ファイルの中身を読む
	HANDLE h = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}

	// アイコンファイルなので大きいファイルサイズは想定しない
	DWORD fileSize = GetFileSize(h, nullptr);
	strm.resize(fileSize);

	DWORD readBytes = 0;
	if (ReadFile(h, &strm.front(), fileSize, &readBytes, nullptr) == FALSE) {
		SPDLOG_ERROR(_T("Failed to ReadFile! err={:x}"), GetLastError());
	}

	CloseHandle(h);

	return true;

}


