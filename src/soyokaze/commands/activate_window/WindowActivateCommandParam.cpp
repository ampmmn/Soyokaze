#include "pch.h"
#include "WindowActivateCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"
#include "commands/validation/CommandEditValidation.h"
#include "resource.h"


using CommandParamErrorCode = launcherapp::commands::validation::CommandParamErrorCode;

namespace launcherapp { namespace commands { namespace activate_window {

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mHotKeyAttr(rhs.mHotKeyAttr),
	mStragegy(rhs.mStragegy),
	mCaptionStr(rhs.mCaptionStr),
	mClassStr(rhs.mClassStr),
	mPlacement(rhs.mPlacement),
	mIsUseRegExp(rhs.mIsUseRegExp),
	mShouldArrangeWindow(rhs.mShouldArrangeWindow),
	mShouldActivateWindow(rhs.mShouldActivateWindow),
	mIsNotifyIfWindowNotFound(rhs.mIsNotifyIfWindowNotFound),
	mIsAllowAutoExecute(rhs.mIsAllowAutoExecute),
	mIsHotKeyOnly(rhs.mIsHotKeyOnly)
{
	auto regCls = rhs.mRegClass.get();
	mRegClass.reset(regCls ? new tregex(*regCls) : nullptr);
	auto regCaption = rhs.mRegCaption.get();
	mRegCaption.reset(regCaption ? new tregex(*regCaption) : nullptr);
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (this != &rhs) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mHotKeyAttr = rhs.mHotKeyAttr;
		mStragegy = rhs.mStragegy;
		mCaptionStr = rhs.mCaptionStr;
		mClassStr = rhs.mClassStr;
		mPlacement = rhs.mPlacement;
		mIsUseRegExp = rhs.mIsUseRegExp;
		mShouldArrangeWindow = rhs.mShouldArrangeWindow;
		mShouldActivateWindow = rhs.mShouldActivateWindow;
		mIsNotifyIfWindowNotFound = rhs.mIsNotifyIfWindowNotFound;
		mIsAllowAutoExecute = rhs.mIsAllowAutoExecute;
		mIsHotKeyOnly = rhs.mIsHotKeyOnly;
		auto regCls = rhs.mRegClass.get();
		mRegClass.reset(regCls ? new tregex(*regCls) : nullptr);
		auto regCaption = rhs.mRegCaption.get();
		mRegCaption.reset(regCaption ? new tregex(*regCaption) : nullptr);
	}
	return *this;
}

bool CommandParam::IsValid(LPCTSTR orgName, int* errCode) const
{
	ASSERT(errCode);

	// 名前チェック
	if (launcherapp::commands::validation::IsValidCommandName(mName, orgName, errCode) == false) {
		return false;
	}

	// 「ウインドウをアクティブにする」と「位置とサイズを変更する」はどちらか一方が有効である必要あり
	if (mShouldArrangeWindow == false && mShouldActivateWindow == false) {
		*errCode = CommandParamErrorCode::ActivateWindow_ActionMustBeSet;
		return false;
	}

	// 
	if (mStragegy == ByClassAndCaption) {
		if (mCaptionStr.IsEmpty() && mClassStr.IsEmpty()) {
			*errCode = CommandParamErrorCode::ActivateWindow_CaptionAndClassBothEmpty;
			return false;
		}

		if (mIsUseRegExp) {
			tregex regExp;
			CString msg;
			if (TryBuildCaptionRegExp(regExp, &msg)  == false) {
				*errCode = CommandParamErrorCode::ActivateWindow_CaptionIsInvalid;
				return false;
			}

			if (TryBuildClassRegExp(regExp, &msg)  == false) {
				*errCode = CommandParamErrorCode::ActivateWindow_ClassIsInvalid;
				return false;
			}
		}
	}

	*errCode = CommandParamErrorCode::Common_NoError;

	return true;
}

bool CommandParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("description"), mDescription);
	entry->Set(_T("WindowSelectionStrategy"), (int)mStragegy);

	entry->Set(_T("CaptionStr"), mCaptionStr);
	entry->Set(_T("ClassStr"), mClassStr);

	entry->SetBytes(_T("Placement"), (const uint8_t*)&mPlacement, sizeof(mPlacement));

	entry->Set(_T("IsUseRegExp"), mIsUseRegExp);
	entry->Set(_T("ShouldArrangeWindow"), mShouldArrangeWindow);
	entry->Set(_T("ShouldActivateWindow"), mShouldActivateWindow);
	entry->Set(_T("IsNotifyIfWindowNotExist"), mIsNotifyIfWindowNotFound);
	entry->Set(_T("IsAllowAutoExecute"), mIsAllowAutoExecute);
	entry->Set(_T("IsHotKeyOnly"), mIsHotKeyOnly);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	CString name = entry->GetName();
	CString descriptionStr = entry->Get(_T("description"), _T(""));

	mStragegy = (WindowSelectionStrategy)entry->Get(_T("WindowSelectionStrategy"), (int)ByClassAndCaption);

	entry->GetBytes(_T("Placement"), (uint8_t*)&mPlacement, sizeof(WINDOWPLACEMENT));


	mName = name;
	mDescription = descriptionStr;
	mCaptionStr = entry->Get(_T("CaptionStr"), _T(""));
	mClassStr = entry->Get(_T("ClassStr"), _T(""));
	mIsUseRegExp = entry->Get(_T("IsUseRegExp"), false);
	mShouldArrangeWindow = entry->Get(_T("ShouldArrangeWindow"), false);
	mShouldActivateWindow = entry->Get(_T("ShouldActivateWindow"), true);
	mIsNotifyIfWindowNotFound = entry->Get(_T("IsNotifyIfWindowNotExist"), false);
	mIsAllowAutoExecute = entry->Get(_T("IsAllowAutoExecute"), false);
	mIsHotKeyOnly = entry->Get(_T("IsHotKeyOnly"), false);

	if (BuildRegExp() == false) {
		return false;
	}

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 
	return true;
}

bool CommandParam::CanFindHwnd() const
{
	return mCaptionStr.IsEmpty() == FALSE || mClassStr.IsEmpty() == FALSE;
}

HWND CommandParam::FindHwnd()
{
	struct local_param {
		static BOOL CALLBACK callback(HWND h, LPARAM lp) {
			auto thisPtr = (local_param*)lp;

			// 自プロセスのウインドウは対象外
			DWORD pid{0};
			GetWindowThreadProcessId(h, &pid);
			if (GetCurrentProcessId() == pid) {
				return TRUE;
			}

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


bool CommandParam::TryBuildRegExp(CString* errMsg) const
{
	tregex regExp;
	if (TryBuildCaptionRegExp(regExp, errMsg) == false) {
		return false;
	}
	if (TryBuildClassRegExp(regExp, errMsg) == false) {
		return false;
	}
	return true;
}

bool CommandParam::BuildCaptionRegExp(CString* errMsg)
{
	if (mStragegy != ByClassAndCaption) {
		// 対象ウインドウの決定方法が「クラス名とキャプションから選択する」でなければ処理不要
		return true;
	}

	auto regExp = std::make_unique<tregex>();
	if (TryBuildCaptionRegExp(*regExp.get(), errMsg) == false) {
		return false;
	}

	mRegCaption.swap(regExp);
	return true;
}

bool CommandParam::TryBuildCaptionRegExp(tregex& regExp, CString* errMsg) const
{
	try {
		if (mIsUseRegExp) {
			regExp = tregex(tstring(mCaptionStr));
		}
	}
	catch(std::regex_error& e) {

		CString msg;
		if (msg.LoadString(IDS_ERR_INVALIDREGEXP) != FALSE) {
			msg += _T("\n");
		}
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
	if (mStragegy != ByClassAndCaption) {
		// 対象ウインドウの決定方法が「クラス名とキャプションから選択する」でなければ処理不要
		return true;
	}

	auto regExp = std::make_unique<tregex>();
	if (TryBuildClassRegExp(*regExp.get(), errMsg) == false) {
		return false;
	}
	mRegClass.swap(regExp);
	return true;
}

bool CommandParam::TryBuildClassRegExp(tregex& regExp, CString* errMsg) const
{
	try {
		if (mIsUseRegExp) {
			regExp = tregex(tstring(mClassStr));
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
		if (mRegCaption.get() == nullptr) {
			if (BuildCaptionRegExp(nullptr) == false) {
				return false;
			}
		}
		try {
			return std::regex_match(tstring(caption), *mRegCaption.get());
		}
		catch(std::regex_error&) {
			spdlog::error("CommandParam::IsMatchCaption regexp is invalid.");
			return false;
		}
	}
	else {
		return mCaptionStr == caption;
	}
}

bool CommandParam::IsMatchClass(LPCTSTR className)
{
	if (IsUseRegExp()) {
		if (mRegClass.get() == nullptr) {
			if (BuildClassRegExp(nullptr) == false) {
				return false;
			}
		}
		try {
			return std::regex_match(tstring(className), *mRegClass.get());
		}
		catch(std::regex_error&) {
			spdlog::error("CommandParam::IsMatchCaption regexp is invalid.");
			return false;
		}
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

