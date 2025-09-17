#include "pch.h"
#include "CopyClipboardAction.h"
#include "commands/common/Clipboard.h"

namespace launcherapp { namespace actions { namespace clipboard {

using namespace launcherapp::commands::common;

CopyAction::CopyAction()
{
}

CopyAction::~CopyAction()
{
}

// Action
// アクションの内容を示す名称
CString CopyAction::GetDisplayName()
{
	return _T("クリップボードにコピー");
}

// アクションを実行する
bool CopyAction::Perform(Parameter* param, String* errMsg)
{
	// クリップボードにコピー
	bool isOK = Clipboard::Copy(param->GetWholeString());
	if (isOK == false && errMsg) {
		*errMsg = "クリップボードのコピーに失敗しました";
		return false;
	}
	return true;
}

}}}

