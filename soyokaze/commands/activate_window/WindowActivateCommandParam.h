#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include <regex>

namespace launcherapp {
namespace commands {
namespace activate_window {

class CommandParam
{
public:
	CommandParam();
	~CommandParam();

	HWND FindHwnd();

	bool BuildRegExp(CString* errMsg = nullptr);
	bool BuildCaptionRegExp(CString* errMsg = nullptr);
	bool BuildClassRegExp(CString* errMsg = nullptr);

	bool IsMatchCaption(LPCTSTR caption);
	bool IsMatchClass(LPCTSTR clsName);

	bool IsUseRegExp() const;
	bool HasCaptionRegExpr() const;
	bool HasClassRegExpr() const;

	bool IsNotifyIfWindowNotFound() const;
public:
	CString mName;
	CString mDescription;

	tregex mRegClass;
	tregex mRegCaption;

	CommandHotKeyAttribute mHotKeyAttr;

	CString mCaptionStr;
	CString mClassStr;
	BOOL mIsUseRegExp;

	// ウインドウが見つからなかった場合に通知
	BOOL mIsNotifyIfWindowNotFound;
};



}
}
}

