#pragma once

class ScopedDCState
{
public:
	ScopedDCState(HDC hdc) : mHdc(hdc)
	{
		mSavedDC = SaveDC(mHdc);
	}
	ScopedDCState(CDC* dc) : mHdc(dc->GetSafeHdc())
	{
		mSavedDC = SaveDC(mHdc);
	}

	~ScopedDCState()
	{
		if (mHdc && mSavedDC >= 0) {
			RestoreDC(mHdc, mSavedDC);
		}
	}

private:
	HDC mHdc{nullptr};
	int mSavedDC{-1};
};
