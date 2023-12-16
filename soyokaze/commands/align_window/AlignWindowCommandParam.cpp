#include "pch.h"
#include "AlignWindowCommandParam.h"
#include "resource.h"

namespace soyokaze {
namespace commands {
namespace align_window {

HWND CommandParam::ITEM::FindHwnd()
{
	struct local_param {
		static BOOL CALLBACK callback(HWND h, LPARAM lp) {
			auto thisPtr = (local_param*)lp;

			LONG_PTR style = GetWindowLongPtr(h, GWL_STYLE);
			LONG_PTR styleRequired = (WS_VISIBLE);
			if ((style & styleRequired) != styleRequired) {
				// 非表示のウインドウは対象外
				return TRUE;
			}

			if (thisPtr->mItem->mCaptionStr.IsEmpty() == FALSE) {
				TCHAR caption[256];
				::GetWindowText(h, caption, 256);
				if (thisPtr->mItem->IsMatchCaption(caption) == FALSE) {
					return TRUE;
				}
			}
			if (thisPtr->mItem->HasClassRegExpr()) {
				TCHAR clsName[256];
				::GetClassName(h, clsName, 256);
				if (thisPtr->mItem->IsMatchClass(clsName) == FALSE) {
					return TRUE;
				}
			}

			thisPtr->mHwnd = h;
			return FALSE;
		}
		CommandParam::ITEM* mItem;
		HWND mHwnd = nullptr;
	} param;
	param.mItem = this;
	EnumWindows(local_param::callback, (LPARAM)&param);

	return param.mHwnd;
}

bool CommandParam::ITEM::BuildRegExp(CString* errMsg)
{
	if (BuildCaptionRegExp(errMsg) == false) {
		return false;
	}
	if (BuildClassRegExp(errMsg) == false) {
		return false;
	}
	return true;
}

bool CommandParam::ITEM::BuildCaptionRegExp(CString* errMsg)
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

bool CommandParam::ITEM::BuildClassRegExp(CString* errMsg)
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

bool CommandParam::ITEM::IsMatchCaption(LPCTSTR caption)
{
	if (mIsUseRegExp) {
		return std::regex_match(tstring(caption), mRegCaption);
	}
	else {
		return mCaptionStr == caption;
	}
}

bool CommandParam::ITEM::IsMatchClass(LPCTSTR className)
{
	if (mIsUseRegExp) {
		return std::regex_match(tstring(className), mRegClass);
	}
	else {
		return mClassStr == className;
	}
}

bool CommandParam::ITEM::HasCaptionRegExpr() const
{
	return mCaptionStr.IsEmpty() == FALSE;
}

bool CommandParam::ITEM::HasClassRegExpr() const
{
	return mClassStr.IsEmpty() == FALSE;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CommandParam::CommandParam() :
	mIsGlobal(false),
	mIsNotifyIfWindowNotFound(FALSE),
	mIsKeepActiveWindow(TRUE)
{
}

CommandParam::~CommandParam()
{
}

}
}
}

