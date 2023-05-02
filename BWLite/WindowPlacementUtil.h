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

// ウインドウ位置情報の保存
bool SavePlacement(CWnd* wnd, LPCTSTR key);
// ウインドウ位置情報の復元
bool LoadPlacement(CWnd* wnd, LPCTSTR key);

