#pragma once

#include "gui/WindowPosition.h"

class MainWindowPosition : public WindowPosition
{
public:
	MainWindowPosition();
	MainWindowPosition(LPCTSTR name);
	~MainWindowPosition();

public:
	int GetHeight();
	void SetHeight(int h);
	bool SyncPosition(HWND hwnd);

protected:
	static void GetFilePath(LPCTSTR baseName, Path& path);
};

