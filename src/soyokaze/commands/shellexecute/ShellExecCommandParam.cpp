#include "pch.h"
#include "ShellExecCommandParam.h"
#include "hotkey/CommandHotKeyManager.h"
#include "resource.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp { namespace commands { namespace shellexecute {


int ATTRIBUTE::GetShowType() const
{
	if (mShowType == 1) {
		return SW_MAXIMIZE;
	}
	else if (mShowType == 2) {
		return SW_SHOWMINIMIZED;
	}
	else {
		return SW_NORMAL;
	}
}

void ATTRIBUTE::SetShowType(int type)
{
	switch(type) {
	case SW_MAXIMIZE:
		mShowType = 1;
		return;
	case SW_SHOWMINIMIZED:
		mShowType = 2;
		return;
	default:
		mShowType = 0;
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

ActivateWindowParam& ActivateWindowParam::operator = (const ActivateWindowParam& rhs)
{
	if (this != &rhs) {
		mCaptionStr = rhs.mCaptionStr;
		mClassStr = rhs.mClassStr;
		mIsEnable = rhs.mIsEnable;
		mIsUseRegExp = rhs.mIsUseRegExp;
	}
	return *this;
}

HWND ActivateWindowParam::FindHwnd()
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
			if (thisPtr->mParam->mClassStr.IsEmpty() == FALSE) {
				TCHAR clsName[256];
				::GetClassName(h, clsName, 256);
				if (thisPtr->mParam->IsMatchClass(clsName) == FALSE) {
					return TRUE;
				}
			}

			thisPtr->mHwnd = h;
			return FALSE;
		}
		ActivateWindowParam* mParam = nullptr;
		HWND mHwnd = nullptr;
	} param;
	param.mParam = this;
	EnumWindows(local_param::callback, (LPARAM)&param);

	return param.mHwnd;
}

bool ActivateWindowParam::IsMatchCaption(LPCTSTR caption)
{
	if (mIsUseRegExp) {
		if (BuildCaptionRegExp(nullptr) == false) {
			return false;
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

bool ActivateWindowParam::IsMatchClass(LPCTSTR clsName)
{
	if (mIsUseRegExp) {
		if (BuildClassRegExp(nullptr) == false) {
			return false;
		}
		try {
			return std::regex_match(tstring(clsName), *mRegClass.get());
		}
		catch(std::regex_error&) {
			spdlog::error("CommandParam::IsMatchCaption regexp is invalid.");
			return false;
		}
	}
	else {
		return mClassStr == clsName;
	}
}

bool ActivateWindowParam::BuildCaptionRegExp(CString* errMsg)
{
	try {
		if (mIsUseRegExp) {
			auto regExp = std::make_unique<tregex>();
			*regExp = tregex(tstring(mCaptionStr));
			mRegCaption.swap(regExp);
		}
		return true;
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
	return false;
}

bool ActivateWindowParam::BuildClassRegExp(CString* errMsg)
{
	try {
		if (mIsUseRegExp) {
			auto regExp = std::make_unique<tregex>();
			*regExp = tregex(tstring(mClassStr));
			mRegClass.swap(regExp);
		}
		return true;
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
		msg += mClassStr;

		if (errMsg) {
			*errMsg = msg;
		}
		return false;
	}
	return false;
}

bool ActivateWindowParam::Save(CommandEntryIF* entry) const
{
	entry->Set(_T("CaptionStr"), mCaptionStr);
	entry->Set(_T("ClassStr"), mClassStr);
	entry->Set(_T("IsEnableWindowActivate"), mIsEnable);
	entry->Set(_T("IsUseRegExp"), mIsUseRegExp);

	return true;
}

bool ActivateWindowParam::Load(CommandEntryIF* entry)
{
	CString captionStr = entry->Get(_T("CaptionStr"), _T(""));
	CString classStr = entry->Get(_T("ClassStr"), _T(""));
	bool isEnable = entry->Get(_T("IsEnableWindowActivate"), false);
	bool isUseRegExp = entry->Get(_T("IsUseRegExp"), false);

	mCaptionStr = captionStr;
	mClassStr = classStr;
	mIsEnable = isEnable;
	mIsUseRegExp = isUseRegExp;

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


CommandParam::CommandParam() :
	mIsRunAsAdmin(FALSE),
	mIsUse0(FALSE),
	mIsUseDescriptionForMatching(FALSE),
	mIsAllowAutoExecute(FALSE)
{
}


CommandParam::CommandParam(const CommandParam& rhs)
{
	mName = rhs.mName;
	mDescription = rhs.mDescription;
	mNormalAttr = rhs.mNormalAttr;
	mNoParamAttr = rhs.mNoParamAttr;
	mIsRunAsAdmin = rhs.mIsRunAsAdmin;
	mIsUse0 = rhs.mIsUse0;
	mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
	mIconData = rhs.mIconData;
	mEnviron = rhs.mEnviron;
	mHotKeyAttr = rhs.mHotKeyAttr;
	mIsAllowAutoExecute = rhs.mIsAllowAutoExecute;
	mActivateWindowParam = rhs.mActivateWindowParam;
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (&rhs != this) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mNormalAttr = rhs.mNormalAttr;
		mNoParamAttr = rhs.mNoParamAttr;
		mIsRunAsAdmin = rhs.mIsRunAsAdmin;
		mIsUse0 = rhs.mIsUse0;
		mIsUseDescriptionForMatching = rhs.mIsUseDescriptionForMatching;
		mIconData = rhs.mIconData;
		mEnviron = rhs.mEnviron;
		mHotKeyAttr = rhs.mHotKeyAttr;
		mIsAllowAutoExecute = rhs.mIsAllowAutoExecute;
		mActivateWindowParam = rhs.mActivateWindowParam;
	}
	return *this;
}

bool CommandParam::Save(CommandEntryIF* entry) const
{
	ASSERT(entry);

	entry->Set(_T("description"), mDescription);
	entry->Set(_T("runas"), mIsRunAsAdmin ? 1 : 0);

	entry->Set(_T("isuse0"), mIsUse0 ? 1 : 0);
	entry->Set(_T("isusedescriptionformatching"), mIsUseDescriptionForMatching ? 1 : 0);

	entry->Set(_T("path"), mNormalAttr.mPath);
	entry->Set(_T("dir"), mNormalAttr.mDir);
	entry->Set(_T("parameter"), mNormalAttr.mParam);
	entry->Set(_T("show"), mNormalAttr.GetShowType());

	entry->Set(_T("path0"), mNoParamAttr.mPath);
	entry->Set(_T("dir0"), mNoParamAttr.mDir);
	entry->Set(_T("parameter0"), mNoParamAttr.mParam);
	entry->Set(_T("show0"), mNoParamAttr.GetShowType());

	entry->SetBytes(_T("IconData"),
	                (const uint8_t*)mIconData.data(), mIconData.size());

	CString key;
	int index = 1;
	for (const auto& item : mEnviron) {
		if (item.first.IsEmpty()) {
			continue;
		}
		key.Format(_T("envname%d"), index);
		entry->Set(key, item.first);
		key.Format(_T("envvalue%d"), index);
		entry->Set(key, item.second);
		index++;
	}

	entry->Set(_T("allow_auto_execute"), mIsAllowAutoExecute ? true : false);

	mActivateWindowParam.Save(entry);

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));

	mIsRunAsAdmin = (entry->Get(_T("runas"), 0) != 0);

	mIsUse0 = entry->Get(_T("isuse0"), 0) != 0;
	mIsUseDescriptionForMatching = entry->Get(_T("isusedescriptionformatching"), 0) != 0;

	mNormalAttr.mPath = entry->Get(_T("path"), _T(""));
	mNormalAttr.mDir = entry->Get(_T("dir"), _T(""));
	mNormalAttr.mParam = entry->Get(_T("parameter"), _T(""));
	mNormalAttr.SetShowType(entry->Get(_T("show"), SW_SHOW));

	mNoParamAttr.mPath = entry->Get(_T("path0"), _T(""));
	mNoParamAttr.mDir = entry->Get(_T("dir0"), _T(""));
	mNoParamAttr.mParam = entry->Get(_T("parameter0"), _T(""));
	mNoParamAttr.SetShowType(entry->Get(_T("show0"), SW_SHOW));

	size_t len = entry->GetBytesLength(_T("IconData"));
	if (len != CommandEntryIF::NO_ENTRY) {
		mIconData.resize(len);
		entry->GetBytes(_T("IconData"), (uint8_t*)mIconData.data(), len);
	}

	// 環境変数
	std::map<CString, CString> envMap;
	CString key;
	CString envName;
	CString envValue;
	for (int index = 1;; ++index) {
		key.Format(_T("envname%d"), index);
		envName = entry->Get(key, _T(""));
		if (envName.IsEmpty()) {
			break;
		}
		key.Format(_T("envvalue%d"), index);
		envValue = entry->Get(key, _T(""));
		envMap[envName] = envValue;
	}
	mEnviron.swap(envMap);

	mIsAllowAutoExecute = entry->Get(_T("allow_auto_execute"), false) ? TRUE : FALSE;

	mActivateWindowParam.Load(entry);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(mName, &mHotKeyAttr); 

	return true;
}

// 
bool CommandParam::IsPathURL() const
{
	if (PathIsURL(mNormalAttr.mPath)) {
		return true;
	}
	if (mIsUse0 == false) {
		return false;
	}
	return PathIsURL(mNoParamAttr.mPath) != FALSE;
}

}}} // end of namespace launcherapp::commands::shellexecute

