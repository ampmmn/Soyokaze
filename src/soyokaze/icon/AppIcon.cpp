#include "pch.h"
#include "AppIcon.h"
#include "icon/IconLoader.h"
#include "utility/AppProfile.h"
#include "utility/Path.h"
#include "resource.h"

namespace launcherapp {
namespace icon {

struct AppIcon::PImpl
{
	Path& GetAppIconPath()
	{
		if (mDefIconPath.get() == nullptr) {
			mDefIconPath.reset(new Path(Path::APPDIR, _T("default.ico")));
		}
		return *mDefIconPath.get();
	}

	std::unique_ptr<Path> mDefIconPath;

	HICON mIconHandle = nullptr;
	bool mIsLoaded = false;
	
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



AppIcon::AppIcon() : in(new PImpl)
{
}

AppIcon::~AppIcon()
{
	if (in->mIconHandle) {
		DestroyIcon(in->mIconHandle);
		in->mIconHandle = nullptr;
	}
}

AppIcon* AppIcon::Get()
{
	static AppIcon inst;
	return &inst;
}

/**
 	デフォルトのアプリアイコンハンドルを取得する
 	@return アイコンハンドル
*/
HICON AppIcon::DefaultIconHandle()
{
	return AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


/**
 	アプリアイコンハンドルを取得する
	利用者側で設定したアイコンがある場合はそのアイコンハンドルを、
	そうでない場合はデフォルトのアプリアイコンハンドルを返す
 	@return アイコンハンドル
*/
HICON AppIcon::IconHandle()
{
	if (in->mIsLoaded) {
		// アイコンがロード済なら
		if (in->mIconHandle) {
			return in->mIconHandle;
		}
		else {
			return DefaultIconHandle();
		}
	}

	in->mIsLoaded = true;

	Path& appIconPath = in->GetAppIconPath();

	if (appIconPath.FileExists() == FALSE) {
		return DefaultIconHandle();
	}

	bool isShared = false;
	HICON h = IconLoader::Get()->LoadIconFromImageFile((LPCTSTR)appIconPath, isShared);
	if (h == nullptr) {
		return DefaultIconHandle();
	}

	in->mIconHandle = h;
	return h;
}

static void CopyBitmapImage(ATL::CImage& imgOut, HBITMAP bmpSrc, int bpp)
{
	BITMAP bm;
	GetObject(bmpSrc, sizeof(BITMAP), &bm);

	imgOut.Create(bm.bmWidth, bm.bmHeight, bpp);

	if (bpp == 1) {
		RGBQUAD colors[2] = { { 0, 0, 0, 0}, { 1, 1, 1, 0 }};
		imgOut.SetColorTable(0, 2, colors);
	}

	HDC dcDst = imgOut.GetDC();
	HDC dc = GetDC(nullptr);
	HDC dcSrc = CreateCompatibleDC(dc);

	auto orgBmp = SelectObject(dcSrc, bmpSrc);
	BitBlt(dcDst, 0, 0, bm.bmWidth, bm.bmHeight, dcSrc, 0, 0, SRCCOPY);
	SelectObject(dcSrc, orgBmp);

	DeleteDC(dcSrc);
	ReleaseDC(nullptr, dcSrc);
	imgOut.ReleaseDC();
}

bool AppIcon::Import(const CString& iconFilePath)
{
	SPDLOG_DEBUG("start");

	// パスが無効ならエラー
	if (PathFileExists(iconFilePath) == FALSE) {
		return false;
	}

	bool isShared = false;
	HICON newIconHandle = IconLoader::Get()->LoadIconFromImageFile(iconFilePath, isShared);
	if (newIconHandle == nullptr) {
		spdlog::warn(_T("Failed to load icon {}"), (LPCTSTR)iconFilePath);
		return false;
	}
	// アイコンハンドルを差し替え
	if (in->mIconHandle) {
		DestroyIcon(in->mIconHandle);
	}
	in->mIconHandle = newIconHandle;
	in->mIsLoaded = true;

	// 次回ロード用に保存
	auto& filePath = in->GetAppIconPath();
	CopyFile(iconFilePath, filePath, FALSE);

	SPDLOG_DEBUG("end");

	return true;
}

void AppIcon::Reset()
{
	if (in->mIsLoaded == false) {
		return;
	}

	if (in->mIconHandle) {
		DestroyIcon(in->mIconHandle);
		in->mIconHandle = nullptr;
	}
	Path& appIconPath = in->GetAppIconPath();
	if (appIconPath.FileExists()) {
		DeleteFile((LPCTSTR)appIconPath);
	}

	in->mIsLoaded = false;
}



}
}

