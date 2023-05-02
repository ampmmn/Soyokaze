#include "pch.h"
#include "framework.h"
#include "WindowPlacementUtil.h"
#include "AppProfile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

WindowPosition::WindowPosition()
{
}

WindowPosition::~WindowPosition()
{
	Save();
}

bool WindowPosition::Restore(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	TCHAR path[32768];
	WindowPosition::GetFilePath(path, 32768);


	CFile file;
	if (file.Open(path, CFile::modeRead | CFile::shareDenyWrite) == FALSE) {
		return false;
	}

	WINDOWPLACEMENT wp;
	file.Read(&wp, sizeof(wp));

	return SetWindowPlacement(hwnd, &wp) != FALSE;
}

bool WindowPosition::Update(HWND hwnd)
{
	WINDOWPLACEMENT wp;
	wp.length = sizeof(wp);
	if (GetWindowPlacement(hwnd, &wp) == false) {
		return false;
	}
	mPosition = wp;
	return true;
}

bool WindowPosition::Save()
{
	try {
		TCHAR path[32768];
		WindowPosition::GetFilePath(path, 32768);

		UINT nFlags = CFile::modeCreate | CFile::modeWrite | CFile::typeBinary;
		CFile file(path, nFlags);
		file.Write(&mPosition, sizeof(mPosition));
		return true;
	}
	catch(...) {
		return false;
	}
}


void WindowPosition::GetFilePath(TCHAR* pathBuf, size_t len)
{
	CAppProfile::GetDirPath(pathBuf, len);
	PathAppend(pathBuf, _T("BWLite.position"));
}

