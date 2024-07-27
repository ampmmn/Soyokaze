#pragma once

class Path;

class WindowPosition
{
public:
	WindowPosition();
	WindowPosition(LPCTSTR name);
	~WindowPosition();

public:
	bool Restore(HWND hwnd);
	bool Update(HWND hwnd);
	bool Save();

protected:
	static void GetFilePath(LPCTSTR baseName, Path& path);

protected:
	CString mName;
	WINDOWPLACEMENT mPosition;
};

