#pragma once

class WindowPosition
{
public:
	WindowPosition();
	~WindowPosition();

public:
	bool Restore(HWND hwnd);
	bool Update(HWND hwnd);
	bool Save();

protected:
	static void GetFilePath(TCHAR* buf, size_t len);

protected:
	WINDOWPLACEMENT mPosition;
};

