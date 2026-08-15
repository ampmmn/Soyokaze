#pragma once

#include <vector>
#include "utility/Regex.h"

namespace launcherapp {
namespace commands {
namespace align_window {

enum ACTION {
	AT_SETPOS,    // サイズ変更
	AT_MAXIMIZE,  // 最大化
	AT_MINIMIZE,  // 最小化
	AT_HIDE,      // 非表示
};

struct ITEM {

	ITEM();

	bool FindHwnd(std::vector<HWND>& windows);
	bool BuildRegExp(CString* errMsg = nullptr);
	bool BuildCaptionRegExp(CString* errMsg = nullptr);
	bool BuildClassRegExp(CString* errMsg = nullptr);

	bool IsMatchCaption(LPCTSTR caption);
	bool IsMatchClass(LPCTSTR clsName);

	bool HasCaptionRegExpr() const;
	bool HasClassRegExpr() const;

	static BOOL CALLBACK OnEnumWindows(HWND h, LPARAM lp);

	int mAction;
	bool mIsUseRegExp;
	bool mIsApplyAll;
	launcherapp::utility::Regex mRegClass;
	launcherapp::utility::Regex mRegCaption;
	WINDOWPLACEMENT mPlacement;
	CString mCaptionStr;
	CString mClassStr;
};

}
}
}

