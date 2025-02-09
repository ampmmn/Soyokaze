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

	HICON h = (HICON)LoadImage(nullptr, (LPCTSTR)appIconPath, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
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
	// パスが無効ならエラー
	if (PathFileExists(iconFilePath) == FALSE) {
		return false;
	}

	bool isShared = false;
	HICON newIconHandle = IconLoader::Get()->LoadIconFromImageFile(iconFilePath, isShared);

	// アイコンハンドルを差し替え
	if (in->mIconHandle) {
		DestroyIcon(in->mIconHandle);
	}
	in->mIconHandle = newIconHandle;
	in->mIsLoaded = true;

	// 次回ロード用に保存
#pragma pack(push, 1)
	struct ICONDIRENTRY {
		BYTE  bWidth;
		BYTE  bHeight;
		BYTE  bColorCount;
		BYTE  bReserved;
		WORD  wPlanes;
		WORD  wBitCount;
		DWORD dwBytesInRes;  // イメージ領域のサイズ
		DWORD dwImageOffset;
	}; 

	struct ICONHEADER {
		WORD idReserved = 0;
		WORD idType = 1;
		WORD idCount = 1;
		ICONDIRENTRY entry[1];
	} iconHdr;
#pragma pack(pop)

	ICONINFO iconInfo;
	if (!GetIconInfo(newIconHandle, &iconInfo)) {
		spdlog::error("Failed to get iconinfo. handle:{}", (void*)newIconHandle);
		return false;
	}

	// アイコンからビットマップ情報を取得
	BITMAP bmpInfo;
	GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmpInfo);
	BITMAP bmpInfoMask;
	GetObject(iconInfo.hbmMask, sizeof(BITMAP), &bmpInfoMask);

	int bpp = bmpInfo.bmBitsPixel <= 24 ? 24 : bmpInfo.bmBitsPixel;

	// イメージデータを取得
	ATL::CImage imgColor;
	CopyBitmapImage(imgColor, iconInfo.hbmColor, bpp);
	ATL::CImage imgMask;
	CopyBitmapImage(imgMask, iconInfo.hbmMask, 1);

	int nImageSizeColor = abs(imgColor.GetPitch() * imgColor.GetHeight());
	int nImageSizeMask =  abs(imgMask.GetPitch() * imgMask.GetHeight());

	// ICOファイルのヘッダーを作成
	auto& entry = iconHdr.entry[0];
	entry.bWidth = (BYTE)bmpInfo.bmWidth;
	entry.bHeight = (BYTE)bmpInfo.bmHeight;
	entry.bColorCount = 0;
	entry.bReserved = 0;
	entry.wPlanes = bmpInfo.bmPlanes;
	entry.wBitCount = bpp;
	entry.dwBytesInRes = sizeof(BITMAPINFOHEADER) + nImageSizeColor + nImageSizeMask;
	entry.dwImageOffset = sizeof(ICONHEADER);

	int width = bmpInfo.bmWidth;
	int height = bmpInfo.bmHeight;

	// ビットマップデータを保存
	auto& filePath = in->GetAppIconPath();
	HANDLE fileHandle = CreateFile(filePath, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (fileHandle == INVALID_HANDLE_VALUE) {
		return false;
	}

	BITMAPINFO bmi = {};
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = width;
	bmi.bmiHeader.biHeight = height * 2;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = bpp;

	DWORD writtenBytes = 0;
	WriteFile(fileHandle, &iconHdr, sizeof(iconHdr), &writtenBytes, nullptr);
	WriteFile(fileHandle, &bmi.bmiHeader, sizeof(BITMAPINFOHEADER), &writtenBytes, nullptr);
	void* ptr = nullptr;

	ptr = imgColor.GetPitch() < 0 ? ((char*)imgColor.GetBits()) + imgColor.GetPitch() * (imgColor.GetHeight()-1)
		                          : ((char*)imgColor.GetBits());
	WriteFile(fileHandle, ptr, nImageSizeColor, &writtenBytes, nullptr);
	
	ptr = imgMask.GetPitch() < 0 ? ((char*)imgMask.GetBits()) + imgMask.GetPitch() * (imgMask.GetHeight() - 1)
	                             : ((char*)imgMask.GetBits());

	WriteFile(fileHandle, ptr, nImageSizeMask, &writtenBytes, nullptr);

	CloseHandle(fileHandle);

	// リソースを解放
	DeleteObject(iconInfo.hbmColor);
	DeleteObject(iconInfo.hbmMask);

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

