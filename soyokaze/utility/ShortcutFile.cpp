#include "pch.h"
#include "framework.h"
#include "ShortcutFile.h"

ShortcutFile::ShortcutFile()
{
}

ShortcutFile::~ShortcutFile()
{
}

bool ShortcutFile::SetLinkPath(LPCTSTR path)
{
	if (PathFileExists(path) == FALSE) {
		return false;
	}
	mLinkPath = path;

	return true;
}

void ShortcutFile::SetArguments(LPCTSTR args)
{
	mArguments = args;
}

void ShortcutFile::SetWorkingDirectory(LPCTSTR dir)
{
	mDir = dir;
}

bool ShortcutFile::Save(LPCTSTR pathToSave)
{
	IShellLink *shellLinkPtr = nullptr;

	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,IID_IShellLink,(void**)&shellLinkPtr);
	if (FAILED(hr)){
		return false;
	}

	shellLinkPtr->SetPath(mLinkPath);
	shellLinkPtr->SetArguments(mArguments);
	shellLinkPtr->SetWorkingDirectory(mDir);

	IPersistFile *persistFilePtr = nullptr;
	hr = shellLinkPtr->QueryInterface(IID_IPersistFile,(void**)&persistFilePtr);
	if(FAILED(hr)){
		shellLinkPtr->Release();
		shellLinkPtr = nullptr;
		return false;
	}

	CStringW pathToSaveW((CString)pathToSave);

	hr = persistFilePtr->Save(pathToSaveW, TRUE);

	persistFilePtr->Release();
	persistFilePtr = nullptr;

	shellLinkPtr->Release();
	shellLinkPtr = nullptr;

	return true;
}

/**
 *  ショートカットのリンク先パス文字列を得る
 *  @return ショートカットが示す実際のファイルへのパス
 *  @param linkPath  ショートカットファイルのパス
 */
CString ShortcutFile::ResolvePath(
	const CString& linkPath
)
{
	IShellLink *shellLinkPtr = nullptr;

	HRESULT hr =
		CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		                 IID_IShellLink, (void**) &shellLinkPtr);

	if (FAILED(hr)){
		return _T("");
	}

	IPersistFile* persistFilePtr = nullptr;
	hr = shellLinkPtr->QueryInterface(IID_IPersistFile, (void**)&persistFilePtr);
	if (FAILED(hr)) {
		shellLinkPtr->Release();
		return _T("");
	}

	hr = persistFilePtr->Load(linkPath, STGM_READ);
	if (FAILED(hr)) {
		shellLinkPtr->Release();
		return _T("");
	}

	wchar_t pathWideChar[32768];

	WIN32_FIND_DATA wfd;
	shellLinkPtr->GetPath(pathWideChar, 32768, &wfd, SLGP_UNCPRIORITY | SLGP_RAWPATH);

	CString path((CStringW)pathWideChar);

	persistFilePtr->Release();
	shellLinkPtr->Release();

	return path;
}

