#include "pch.h"
#include "AlignWindowCommandParam.h"
#include "resource.h"

namespace launcherapp {
namespace commands {
namespace align_window {

CommandParam::ITEM::ITEM() : mIsUseRegExp(FALSE), mIsApplyAll(FALSE), mAction(AT_SETPOS)
{
}

bool CommandParam::ITEM::FindHwnd(std::vector<HWND>& windows)
{
	struct local_param {
		static BOOL CALLBACK callback(HWND h, LPARAM lp) {
			auto thisPtr = (local_param*)lp;

			LONG_PTR style = GetWindowLongPtr(h, GWL_STYLE);
			LONG_PTR styleRequired = 0;
			if ((style & styleRequired) != styleRequired) {
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

			thisPtr->mHwnd.push_back(h);

			if (thisPtr->mItem->mIsApplyAll) {
				// 「複数該当する場合はすべて適用」の場合は検索を継続する
				return TRUE;
			}
			return FALSE;
		}
		CommandParam::ITEM* mItem;
		std::vector<HWND> mHwnd;
	} param;
	param.mItem = this;
	EnumWindows(local_param::callback, (LPARAM)&param);

	windows.swap(param.mHwnd);
	return windows.size() > 0;
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
	if (mCaptionStr.IsEmpty()) {
		// 設定値がない場合は無条件で許可
		return true;
	}
	if (mIsUseRegExp) {
		return std::regex_match(tstring(caption), mRegCaption);
	}
	else {
		return mCaptionStr == caption;
	}
}

bool CommandParam::ITEM::IsMatchClass(LPCTSTR className)
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

