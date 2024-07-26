#include "pch.h"
#include "AlignWindowCommandParamItem.h"
#include <resource.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace align_window {

ITEM::ITEM() :
	mIsUseRegExp(false), mIsApplyAll(false), mAction(AT_SETPOS), mPlacement({})
{
}

struct CALLBACK_PARAM
{
	CALLBACK_PARAM(ITEM* thisPtr) : mItem(thisPtr) {}

	ITEM* mItem;
	std::vector<HWND> mHwnd;
};

BOOL ITEM::OnEnumWindows(HWND h, LPARAM lp)
{
	auto param = (CALLBACK_PARAM*)lp;
	auto thisPtr = param->mItem;

	LONG_PTR style = GetWindowLongPtr(h, GWL_STYLE);
	LONG_PTR styleRequired = 0;
	if ((style & styleRequired) != styleRequired) {
		return TRUE;
	}

	if (thisPtr->mCaptionStr.IsEmpty() == FALSE) {
		TCHAR caption[256];
		::GetWindowText(h, caption, 256);
		if (thisPtr->IsMatchCaption(caption) == FALSE) {
			return TRUE;
		}
	}
	if (thisPtr->HasClassRegExpr()) {
		TCHAR clsName[256];
		::GetClassName(h, clsName, 256);
		if (thisPtr->IsMatchClass(clsName) == FALSE) {
			return TRUE;
		}
	}

	try {
		param->mHwnd.push_back(h);
	}
	catch(...) {
		return FALSE;
	}

	if (thisPtr->mIsApplyAll) {
		// 「複数該当する場合はすべて適用」の場合は検索を継続する
		return TRUE;
	}
	return FALSE;
}

bool ITEM::FindHwnd(std::vector<HWND>& windows)
{
	struct CALLBACK_PARAM param(this);
	EnumWindows(OnEnumWindows, (LPARAM)&param);

	windows.swap(param.mHwnd);
	return windows.size() > 0;
}

bool ITEM::BuildRegExp(CString* errMsg)
{
	if (BuildCaptionRegExp(errMsg) == false) {
		return false;
	}
	if (BuildClassRegExp(errMsg) == false) {
		return false;
	}
	return true;
}

bool ITEM::BuildCaptionRegExp(CString* errMsg)
{
	try {
		if (mIsUseRegExp) {
			mRegCaption = tregex(tstring(mCaptionStr));
		}
	}
	catch(std::regex_error& e) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
		msg += _T("\n");

		CStringA what(e.what());
		msg += _T("\n");
		msg += (CString)what;
		msg += _T("\n");
		msg += mCaptionStr;

		if (errMsg) {
			*errMsg = msg;
		}
		return false;
	}
	return true;
}

bool ITEM::BuildClassRegExp(CString* errMsg)
{
	try {
		if (mIsUseRegExp) {
			mRegClass = tregex(tstring(mClassStr));
		}
	}
	catch(std::regex_error& e) {
		CString msg((LPCTSTR)IDS_ERR_INVALIDREGEXP);
		msg += _T("\n");

		CStringA what(e.what());
		msg += _T("\n");
		msg += (CString)what;
		msg += _T("\n");
		msg += mClassStr;

		if (errMsg) {
			*errMsg = msg;
		}
		return false;
	}
	return true;
}

bool ITEM::IsMatchCaption(LPCTSTR caption)
{
	if (mCaptionStr.IsEmpty()) {
		// 設定値がない場合は無条件で許可
		return true;
	}
	try {
		if (mIsUseRegExp) {
			return std::regex_match(tstring(caption), mRegCaption);
		}
		else {
			return mCaptionStr == caption;
		}
	}
	catch(std::regex_error&) {
		return false;
	}
}

bool ITEM::IsMatchClass(LPCTSTR className)
{
	if (mClassStr.IsEmpty()) {
		// 設定値がない場合は無条件で許可
		return true;
	}
	if (mIsUseRegExp) {
		return std::regex_match(tstring(className), mRegClass);
	}
	else {
		return mClassStr == className;
	}
}

bool ITEM::HasCaptionRegExpr() const
{
	return mCaptionStr.IsEmpty() == FALSE;
}

bool ITEM::HasClassRegExpr() const
{
	return mClassStr.IsEmpty() == FALSE;
}

}
}
}

