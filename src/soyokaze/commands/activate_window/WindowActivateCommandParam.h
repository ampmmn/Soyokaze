#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"
#include <regex>

namespace launcherapp { namespace commands { namespace activate_window {

class CommandParam
{
public:
	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);

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
	bool mIsUseRegExp{false};

	// ウインドウが見つからなかった場合に通知
	bool mIsNotifyIfWindowNotFound{false};
	// 自動実行を許可するか?
	bool mIsAllowAutoExecute{false};

	bool mIsHotKeyOnly{false};
};



}}}

