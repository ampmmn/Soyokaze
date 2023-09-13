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
 *  @param description 説明(任意)
 */
CString ShortcutFile::ResolvePath(
	const CString& linkPath,
	CString* description

)
{
	CComPtr<IShellLink> shellLinkPtr;

	HRESULT hr =
		CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
		                 IID_IShellLink, (void**) &shellLinkPtr);

	if (FAILED(hr)){
		return _T("");
	}

	CComPtr<IPersistFile> persistFilePtr;
	hr = shellLinkPtr->QueryInterface(IID_IPersistFile, (void**)&persistFilePtr);
	if (FAILED(hr)) {
		return _T("");
	}

	hr = persistFilePtr->Load(linkPath, STGM_READ);
	if (FAILED(hr)) {
		return _T("");
	}

	wchar_t pathWideChar[MAX_PATH_NTFS];

	WIN32_FIND_DATA wfd;
	shellLinkPtr->GetPath(pathWideChar, MAX_PATH_NTFS, &wfd, 0);

	if (description) {
		shellLinkPtr->GetDescription(description->GetBuffer(INFOTIPSIZE), INFOTIPSIZE);
		description->ReleaseBuffer();
	}

	CString path((CStringW)pathWideChar);
	return path;
}

