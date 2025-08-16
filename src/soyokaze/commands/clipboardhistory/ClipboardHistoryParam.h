#pragma once

#include "setting/Settings.h"


namespace launcherapp { namespace commands { namespace clipboardhistory {

class Param
{
public:
	bool Save(Settings& settings) const
	{
		settings.Set(_T("ClipboardHistory:IsEnable"), mIsEnable);
		settings.Set(_T("ClipboardHistory:Prefix"), mPrefix);
		settings.Set(_T("ClipboardHistory:NumOfResults"), mNumOfResults);
		settings.Set(_T("ClipboardHistory:SizeLimit"), mSizeLimit);
		settings.Set(_T("ClipboardHistory:CountLimit"), mCountLimit);
		settings.Set(_T("ClipboardHistory:Interval"), mInterval);
		settings.Set(_T("ClipboardHistory:ExcludePattern"), mExcludePattern);
		settings.Set(_T("ClipboardHistory:IsDisableMigemo"), mIsDisableMigemo);
		settings.Set(_T("ClipboardHistory:UsePreview"), mUsePreview);
		return true;
	}

	bool Load(Settings& settings)
	{
		mIsEnable = settings.Get(_T("ClipboardHistory:IsEnable"), false);
		mPrefix = settings.Get(_T("ClipboardHistory:Prefix"), _T("cb"));
		mNumOfResults = settings.Get(_T("ClipboardHistory:NumOfResults"), 16);
		mSizeLimit = settings.Get(_T("ClipboardHistory:SizeLimit"), 4);
		mCountLimit = settings.Get(_T("ClipboardHistory:CountLimit"), 1024);
		mInterval = settings.Get(_T("ClipboardHistory:Interval"), 500);
		mExcludePattern = settings.Get(_T("ClipboardHistory:ExcludePattern"), _T(""));
		mIsDisableMigemo = settings.Get(_T("ClipboardHistory:IsDisableMigemo"), true);
		mUsePreview = settings.Get(_T("ClipboardHistory:UsePreview"), false);
		return true;
	}

	CString mPrefix{_T("cb")};
	CString mExcludePattern;
	int mNumOfResults{16};
	int mSizeLimit{4};
	int mCountLimit{1024};
	int mInterval{500};
	bool mIsEnable{false};
	bool mIsDisableMigemo{true};
	bool mUsePreview{false};
};


}}}

