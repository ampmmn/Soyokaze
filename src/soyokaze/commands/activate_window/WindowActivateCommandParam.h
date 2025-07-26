#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/core/CommandEntryIF.h"
#include <memory>
#include <regex>

namespace launcherapp { namespace commands { namespace activate_window {

class CommandParam
{
public:
	CommandParam() = default;
	CommandParam(const CommandParam& rhs);
	~CommandParam();

	CommandParam& operator = (const CommandParam& rhs);

	bool IsValid(LPCTSTR orgName, int* errCode) const;

	bool Save(CommandEntryIF* entry) const;
	bool Load(CommandEntryIF* entry);


	bool CanFindHwnd() const;
	HWND FindHwnd();

	bool BuildRegExp(CString* errMsg = nullptr);
	bool TryBuildRegExp(CString* errMsg = nullptr) const;
	bool BuildCaptionRegExp(CString* errMsg = nullptr);
	bool TryBuildCaptionRegExp(tregex& regExp, CString* errMsg = nullptr) const;
	bool BuildClassRegExp(CString* errMsg = nullptr);
	bool TryBuildClassRegExp(tregex& regExp, CString* errMsg = nullptr) const;

	bool IsMatchCaption(LPCTSTR caption);
	bool IsMatchClass(LPCTSTR clsName);

	bool IsUseRegExp() const;
	bool HasCaptionRegExpr() const;
	bool HasClassRegExpr() const;

	bool IsNotifyIfWindowNotFound() const;



public:
	CString mName;
	CString mDescription;

	CommandHotKeyAttribute mHotKeyAttr;

	CString mCaptionStr;
	CString mClassStr;
	bool mIsUseRegExp{false};

	// ウインドウが見つからなかった場合に通知
	bool mIsNotifyIfWindowNotFound{false};
	// 自動実行を許可するか?
	bool mIsAllowAutoExecute{false};

	bool mIsHotKeyOnly{false};

private:
	std::unique_ptr<tregex> mRegClass;
	std::unique_ptr<tregex> mRegCaption;

};



}}}

