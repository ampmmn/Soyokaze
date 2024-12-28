#pragma once

class HWNDRect
{
public:
	HWNDRect(HWND hwnd);
	~HWNDRect();

	operator RECT*() { return &mRect; }
	operator const RECT*() const { return &mRect; }
	RECT* operator ->() noexcept { return &mRect; }

	void MapTo(HWND hwnd);
	void MapToParent();

	int Width() const;
	int Height() const;

	int WindowWidth() const;
	int WindowHeight() const;

private:
	RECT mRect;
	HWND mHwnd;
};

