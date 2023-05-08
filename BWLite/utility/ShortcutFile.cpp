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
	IShellLink *shellLinkPtr;

	HRESULT hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,IID_IShellLink,(LPVOID *)&shellLinkPtr);
	if (FAILED(hr)){
		return false;
	}

	shellLinkPtr->SetPath(mLinkPath);
	shellLinkPtr->SetArguments(mArguments);
	shellLinkPtr->SetWorkingDirectory(mDir);

	IPersistFile *persistFilePtr;
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
