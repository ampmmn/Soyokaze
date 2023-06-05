#pragma once

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
	void GetFilePath(TCHAR* buf, size_t len);
	static void GetFilePath(LPCTSTR baseName, TCHAR* buf, size_t len);

protected:
	CString mName;
	WINDOWPLACEMENT mPosition;
};

