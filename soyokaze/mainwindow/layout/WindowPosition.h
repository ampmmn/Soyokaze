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
	bool UpdateExceptHeight(HWND hwnd);
	bool Save();

	WINDOWPLACEMENT GetPosition() const;

	bool SetPositionTemporary(HWND hwnd, const CRect& rc);
	bool SyncPosition(HWND hwnd);

protected:
	static void GetFilePath(LPCTSTR baseName, Path& path);

protected:
	CString mName;
	WINDOWPLACEMENT mPosition;
	bool mIsLoaded;
};

