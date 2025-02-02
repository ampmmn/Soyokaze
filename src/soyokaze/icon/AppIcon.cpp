#include "pch.h"
#include "AppIcon.h"
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

bool AppIcon::Import(const CString& iconFilePath)
{
	// パスが無効ならエラー
	if (PathFileExists(iconFilePath) == FALSE) {
		return false;
	}

	// アイコンとしてロードできるかチェック
	HICON h = (HICON)LoadImage(nullptr, iconFilePath, IMAGE_ICON, 32, 32, LR_LOADFROMFILE);
	if (h == nullptr) {
		return false;
	}

	// アイコンハンドルを差し替え
	if (in->mIconHandle) {
		DestroyIcon(in->mIconHandle);
	}
	in->mIconHandle = h;
	in->mIsLoaded = true;

	// 次回ロード用に保存
	BOOL isOK = CopyFile(iconFilePath, (LPCTSTR)in->GetAppIconPath(), FALSE);
	if (isOK == FALSE) {
		spdlog::warn("Failed to import defaulticon.");
	}
	return isOK;

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

