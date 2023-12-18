#pragma once

#include "HotKeyAttribute.h"
#include <regex>
#include <vector>

namespace soyokaze {
namespace commands {
namespace align_window {

class CommandParam
{
public:
	enum ACTION {
		AT_SETPOS,    // サイズ変更
		AT_MAXIMIZE,  // 最大化
		AT_MINIMIZE,  // 最小化
	};

	struct ITEM {

		HWND FindHwnd();
		bool BuildRegExp(CString* errMsg = nullptr);
		bool BuildCaptionRegExp(CString* errMsg = nullptr);
		bool BuildClassRegExp(CString* errMsg = nullptr);

		bool IsMatchCaption(LPCTSTR caption);
		bool IsMatchClass(LPCTSTR clsName);

		bool HasCaptionRegExpr() const;
		bool HasClassRegExpr() const;

		CString mCaptionStr;
		CString mClassStr;
		BOOL mIsUseRegExp;

		POINT mPos;
		SIZE mSize;
		int mAction;

		tregex mRegClass;
		tregex mRegCaption;
	};

public:
	CommandParam();
	~CommandParam();

public:
	CString mName;
	CString mDescription;

	HOTKEY_ATTR mHotKeyAttr;
	bool mIsGlobal;

	// ウインドウが見つからなかった場合に通知
	BOOL mIsNotifyIfWindowNotFound;
	// アクティブなウインドウを実行後も変えない
	BOOL mIsKeepActiveWindow;

	std::vector<ITEM> mItems;
};



}
}
}

