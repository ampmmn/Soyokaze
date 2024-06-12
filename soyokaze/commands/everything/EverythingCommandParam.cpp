#include "pch.h"
#include "EverythingCommandParam.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace everything {

CommandParam::CommandParam()
{
}


CommandParam::~CommandParam()
{
}

CString CommandParam::BuildQueryString(const CString& queryStr)
{
	// コマンドに設定されたオプションをEverythingの検索キーワードに置換する
	CString builtString;
	switch(mTargetType) {
	case 1:
		builtString += _T("file: ");
		break;
	case 2:
		builtString += _T("folder: ");
		break;
	case 0:   // through
	default:
		// なし
		break;
	}

	bool hasSpace = (mBaseDir.Find(_T(" ")) != -1);

	if (hasSpace) {
		builtString += _T('"');
		builtString += mBaseDir;
		builtString += _T('"');
	}
	else {
		builtString += mBaseDir;
	}
	builtString += _T(" ");

	builtString += mOtherParam;

	if (mIsMatchCase) {
		builtString += _T("case:");
	}
	if (mIsRegex) {
		builtString += _T("regex:");
	}

	return builtString + queryStr;
}

} // end of namespace everything
} // end of namespace commands
} // end of namespace launcherapp

