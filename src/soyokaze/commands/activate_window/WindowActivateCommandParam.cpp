#include "pch.h"
#include "WindowActivateCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"
#include "resource.h"

namespace launcherapp { namespace commands { namespace activate_window {

bool CommandParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("description"), mDescription);

	entry->Set(_T("CaptionStr"), mCaptionStr);
	entry->Set(_T("ClassStr"), mClassStr);
	entry->Set(_T("IsUseRegExp"), mIsUseRegExp);
	entry->Set(_T("IsNotifyIfWindowNotExist"), mIsNotifyIfWindowNotFound);
	entry->Set(_T("IsAllowAutoExecute"), mIsAllowAutoExecute);
	entry->Set(_T("IsHotKeyOnly"), mIsHotKeyOnly);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	CString name = entry->GetName();
	CString descriptionStr = entry->Get(_T("description"), _T(""));

	CString captionStr = entry->Get(_T("CaptionStr"), _T(""));
	CString classStr = entry->Get(_T("ClassStr"), _T(""));
	bool isUseRegExp = entry->Get(_T("IsUseRegExp"), false);
	bool isNotify = entry->Get(_T("IsNotifyIfWindowNotExist"), false);
	bool isAutoExec = entry->Get(_T("IsAllowAutoExecute"), false);
	bool isHotKeyOnly = entry->Get(_T("IsHotKeyOnly"), false);

	mName = name;
	mDescription = descriptionStr;
	mCaptionStr = captionStr;
	mClassStr = classStr;
	mIsUseRegExp = isUseRegExp;
	mIsNotifyIfWindowNotFound = isNotify;
	mIsAllowAutoExecute = isAutoExec;
	mIsHotKeyOnly = isHotKeyOnly;

	if (BuildRegExp() == false) {
		return false;
	}

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 
	return true;
}

HWND CommandParam::FindHwnd()
{
	struct local_param {
		static BOOL CALLBACK callback(HWND h, LPARAM lp) {
			auto thisPtr = (local_param*)lp;

			if (thisPtr->mParam->mCaptionStr.IsEmpty() == FALSE) {
				TCHAR caption[256];
				::GetWindowText(h, caption, 256);
				if (thisPtr->mParam->IsMatchCaption(caption) == FALSE) {
					return TRUE;
				}
			}
			if (thisPtr->mParam->HasClassRegExpr()) {
				TCHAR clsName[256];
				::GetClassName(h, clsName, 256);
				if (thisPtr->mParam->IsMatchClass(clsName) == FALSE) {
					return TRUE;
				}
			}

			thisPtr->mHwnd = h;
			return FALSE;
		}
		CommandParam* mParam = nullptr;
		HWND mHwnd = nullptr;
	} param;
	param.mParam = this;
	EnumWindows(local_param::callback, (LPARAM)&param);

	return param.mHwnd;
}

bool CommandParam::BuildRegExp(CString* errMsg)
{
	if (BuildCaptionRegExp(errMsg) == false) {
		return false;
	}
	if (BuildClassRegExp(errMsg) == false) {
		return false;
	}
	return true;
}

bool CommandParam::BuildCaptionRegExp(CString* errMsg)
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

bool CommandParam::BuildClassRegExp(CString* errMsg)
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

bool CommandParam::IsMatchCaption(LPCTSTR caption)
{
	if (IsUseRegExp()) {
		return std::regex_match(tstring(caption), mRegCaption);
	}
	else {
		return mCaptionStr == caption;
	}
}

bool CommandParam::IsMatchClass(LPCTSTR className)
{
	if (IsUseRegExp()) {
		return std::regex_match(tstring(className), mRegClass);
	}
	else {
		return mClassStr == className;
	}
}

bool CommandParam::IsUseRegExp() const
{
	return mIsUseRegExp;
}

bool CommandParam::HasCaptionRegExpr() const
{
	return mCaptionStr.IsEmpty() == FALSE;
}

bool CommandParam::HasClassRegExpr() const
{
	return mClassStr.IsEmpty() == FALSE;
}

bool CommandParam::IsNotifyIfWindowNotFound() const
{
	return mIsNotifyIfWindowNotFound;
}

}}}

