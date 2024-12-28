#include "pch.h"
#include "HWNDRect.h"
#include <algorithm>

HWNDRect::HWNDRect(HWND hwnd) : mHwnd(hwnd)
{
	GetClientRect(hwnd, &mRect);
}

HWNDRect::~HWNDRect()
{
}

void HWNDRect::MapTo(HWND hwnd)
{
	POINT points[2] = {
		{ mRect.left, mRect.top },
		{ mRect.right, mRect.bottom },
	};
	MapWindowPoints(mHwnd, hwnd, points, 2);

	RECT rc;
	rc.left = (std::min)(points[0].x, points[1].x);
	rc.top = (std::min)(points[0].y, points[1].y);
	rc.right = (std::max)(points[0].x, points[1].x);
	rc.bottom = (std::max)(points[0].y, points[1].y);

	std::swap(rc, mRect);
	mHwnd = hwnd;
}

void HWNDRect::MapToParent()
{
	MapTo(GetParent(mHwnd));
}

int HWNDRect::Width() const
{
	return mRect.right - mRect.left;
}

int HWNDRect::Height() const
{
	return mRect.bottom - mRect.top;
}

int HWNDRect::WindowWidth() const
{
	RECT rc;
	GetWindowRect(mHwnd, &rc);
	return rc.right - rc.left;
}

int HWNDRect::WindowHeight() const
{
	RECT rc;
	GetWindowRect(mHwnd, &rc);
	return rc.bottom - rc.top;
}
