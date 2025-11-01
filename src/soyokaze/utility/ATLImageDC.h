#pragma once

#include "ScopedDCState.h"
#include <atlimage.h>
#include <memory>

class ATLImageDC
{
public:
	ATLImageDC(ATL::CImage& image) : mImage(&image)
	{
		mDc = CDC::FromHandle(mImage->GetDC());
		mScopedDCState.reset(new ScopedDCState(mDc));
	}
	~ATLImageDC()
 	{
		mScopedDCState.reset();
		mImage->ReleaseDC();
	}
	CDC* operator -> () { return mDc; }
	operator HDC() { return mDc->GetSafeHdc(); }

private:
	ATL::CImage* mImage{ nullptr };
	CDC* mDc{nullptr};
	std::unique_ptr<ScopedDCState> mScopedDCState;

};
