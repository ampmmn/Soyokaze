#include "pch.h"
#include "framework.h"
#include "CmdReceiveEdit.h"
#include "SharedHwnd.h"
#include "mainwindow/interprocess/InterProcessMessageQueue.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr int TIMERID_READQUEUE = 1;

using namespace launcherapp::mainwindow::interprocess;

CmdReceiveEdit::CmdReceiveEdit()
{
}

CmdReceiveEdit::~CmdReceiveEdit()
{
}

BEGIN_MESSAGE_MAP(CmdReceiveEdit, CEdit)
	ON_WM_CREATE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CmdReceiveEdit::Init()
{
	::SetTimer(GetSafeHwnd(), TIMERID_READQUEUE, 50, 0);
}


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

void CmdReceiveEdit::OnTimer(UINT_PTR timerId)
{
	if (timerId != TIMERID_READQUEUE) {
		return;
	}


	EVENT_ID id;
	std::vector<uint8_t> stm;

	auto queue = InterProcessMessageQueue::GetInstance();
	if (queue->Dequeue(&id, stm) == false) {
		return;
	}

	if (id == SEND_COMMAND) {
		auto param = (SEND_COMMAND_PARAM*)stm.data();
		OnSendCommand(param);
		return;
	}
	else if (id == SET_CARETRANGE) {
		auto param = (SET_CARETRANGE_PARAM*)stm.data();
		OnSetCaretRange(param);
		return;
	}
	else if (id == CHANGE_DIRECTORY) {
		auto param = (CHANGE_DIRECTORY_PARAM*)stm.data();
		OnChangeDirectory(param);
		return;
	}
	else if (id == HIDE) {
		SharedHwnd sharedHwnd;
		HWND hwnd = sharedHwnd.GetHwnd();
		if (IsWindow(hwnd) == FALSE) {
			return ;
		}
		::PostMessage(hwnd, WM_APP+7, 0, 0);
		return;
	}
	else if (id == ACTIVATE_WINDOW) {
		// 先行プロセスを有効化する
		SharedHwnd sharedHwnd;
		HWND hwnd = sharedHwnd.GetHwnd();
		if (IsWindow(hwnd) == FALSE) {
			return;
		}
		::PostMessage(hwnd, WM_APP+2, 0, 0);
		return;
	}
}
