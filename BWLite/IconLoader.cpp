#include "pch.h"
#include "framework.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct IconLoader::PImpl
{
	PImpl() : 
		mFolderIcon(nullptr),
		mWebIcon(nullptr),
		mNewIcon(nullptr),
		mExitIcon(nullptr),
		mEditIcon(nullptr),
		mSettingIcon(nullptr),
		mKeywordManagerIcon(nullptr),
		mDefaultIcon(nullptr),
		mReloadIcon(nullptr),
		mUserDirIcon(nullptr),
		mRegisterWindowIcon(nullptr),
		mUnknownIcon(nullptr)
	{
		const LPCTSTR SYSTEMROOT = _T("SystemRoot");

		size_t reqLen = 0;
		_tgetenv_s(&reqLen, mImgResDll, 32768, SYSTEMROOT);
		PathAppend(mImgResDll, _T("System32"));
		PathAppend(mImgResDll, _T("imageres.dll"));

	}

	HICON GetImageResIcon(int index)
	{
		HICON icon[1];
		UINT n = ExtractIconEx(mImgResDll, index, icon, NULL, 1);
		return (n == 1) ? icon[0]: NULL;
	}

	TCHAR mImgResDll[32768];
	HICON mFolderIcon;
	HICON mWebIcon;
	HICON mNewIcon;
	HICON mExitIcon;
	HICON mEditIcon;
	HICON mSettingIcon;
	HICON mKeywordManagerIcon;
	HICON mDefaultIcon;
	HICON mReloadIcon;
	HICON mUserDirIcon;
	HICON mRegisterWindowIcon;
	HICON mUnknownIcon;
};

IconLoader::IconLoader() : in(new PImpl)
{
}

IconLoader::~IconLoader()
{
	delete in;
}

IconLoader* IconLoader::Get()
{
	static IconLoader instance;
	return &instance;
}

HICON IconLoader::LoadIconFromPath(const CString& path)
{
	if (PathIsDirectory(path)) {
		return LoadFolderIcon();
	}
	if (PathFileExists(path)) {
		SHFILEINFO sfi = {};
		HIMAGELIST hImgList =
			(HIMAGELIST)::SHGetFileInfo(path, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_LARGEICON | SHGFI_USEFILEATTRIBUTES);
		HICON hIcon = sfi.hIcon;
		return hIcon;
	}
	if (path.Find(_T("http")) == 0) {
		return LoadWebIcon();
	}
	return LoadUnknownIcon();
}

HICON IconLoader::LoadFolderIcon()
{
	if (in->mFolderIcon) {
		return in->mFolderIcon;
	}
	in->mFolderIcon = in->GetImageResIcon(3);
	return in->mFolderIcon;
}

HICON IconLoader::LoadWebIcon()
{
	if (in->mWebIcon) {
		return in->mWebIcon;
	}
	in->mWebIcon = in->GetImageResIcon(20);
	return in->mWebIcon;
}

HICON IconLoader::LoadNewIcon()
{
	// ToDo: 実装
	return AfxGetApp()->LoadIcon(IDI_ICON2);
}

HICON IconLoader::LoadSettingIcon()
{
	if (in->mSettingIcon) {
		return in->mSettingIcon;
	}

	in->mSettingIcon = in->GetImageResIcon(109);
	return in->mSettingIcon;
}

HICON IconLoader::LoadExitIcon()
{
	in->mExitIcon = in->GetImageResIcon(235);
	return in->mExitIcon;
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
	if (in->mUserDirIcon) {
		return in->mUserDirIcon;
	}

	in->mUserDirIcon = in->GetImageResIcon(157);
	return in->mUserDirIcon;
}

HICON IconLoader::LoadMainDirIcon()
{
	if (in->mUserDirIcon) {
		return in->mUserDirIcon;
	}

	in->mUserDirIcon = in->GetImageResIcon(157);
	return in->mUserDirIcon;
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
	if (in->mUnknownIcon) {
		return in->mUnknownIcon;
	}

	in->mUnknownIcon = in->GetImageResIcon(2);
	return in->mUnknownIcon;
}

HICON IconLoader::LoadReloadIcon()
{
	if (in->mReloadIcon) {
		return in->mReloadIcon;
	}
	in->mReloadIcon = in->GetImageResIcon(228);
	return in->mReloadIcon;
}

HICON IconLoader::LoadRegisterWindowIcon()
{
	if (in->mRegisterWindowIcon) {
		return in->mRegisterWindowIcon;
	}
	in->mRegisterWindowIcon = in->GetImageResIcon(19);
	return in->mRegisterWindowIcon;
}

