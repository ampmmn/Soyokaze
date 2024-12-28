#include "pch.h"
#include "framework.h"
#include "CmdReceiveEdit.h"
#include "mainwindow/interprocess/InterProcessEventID.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::mainwindow::interprocess;

CmdReceiveEdit::CmdReceiveEdit()
{
}

CmdReceiveEdit::~CmdReceiveEdit()
{
}

BEGIN_MESSAGE_MAP(CmdReceiveEdit, CEdit)
	ON_WM_COPYDATA()
END_MESSAGE_MAP()


BOOL CmdReceiveEdit::OnSendCommand(SEND_COMMAND_PARAM* param)
{
	if (param->mIsPasteOnly) {
		// テキスト設定のみ
		GetParent()->SendMessage(WM_APP + 11, 0, (LPARAM)param->mText);
	}
	else {
		// コマンド実行
		GetParent()->SendMessage(WM_APP + 3, 0, (LPARAM)param->mText);
	}
	return TRUE;
}

BOOL CmdReceiveEdit::OnSetCaretRange(SET_CARETRANGE_PARAM* param)
{
	GetParent()->SendMessage(WM_APP + 12, param->mStartPos, param->mLength);
	return TRUE;
}

BOOL CmdReceiveEdit::OnChangeDirectory(CHANGE_DIRECTORY_PARAM* param)
{
	SPDLOG_DEBUG(_T("path:{}"), (LPCTSTR)param->mDirPath);
	return SetCurrentDirectory(param->mDirPath);
}

BOOL CmdReceiveEdit::OnCopyData(CWnd*, COPYDATASTRUCT* data)
{
	if (data->dwData == SEND_COMMAND) {
		auto param = (SEND_COMMAND_PARAM*)data->lpData;
		return OnSendCommand(param);
	}
	else if (data->dwData == SET_CARETRANGE) {
		auto param = (SET_CARETRANGE_PARAM*)data->lpData;
		return OnSetCaretRange(param);
	}
	else if (data->dwData == CHANGE_DIRECTORY) {
		auto param = (CHANGE_DIRECTORY_PARAM*)data->lpData;
		return OnChangeDirectory(param);
	}

	return FALSE;
}

