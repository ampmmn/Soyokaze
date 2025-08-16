#pragma once

#include "gui/WindowPosition.h"

class MainWindowPosition : public WindowPosition
{
public:
	MainWindowPosition();
	MainWindowPosition(LPCTSTR name);
	~MainWindowPosition();

public:
	bool UpdateExceptHeight(HWND hwnd);
	bool SetPositionTemporary(HWND hwnd, const CRect& rc);
	bool SyncPosition(HWND hwnd);

protected:
	static void GetFilePath(LPCTSTR baseName, Path& path);
};

