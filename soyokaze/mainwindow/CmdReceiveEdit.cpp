#include "pch.h"
#include "framework.h"
#include "CmdReceiveEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CmdReceiveEdit::CmdReceiveEdit() : mIsPasteOnly(false)
{
}

CmdReceiveEdit::~CmdReceiveEdit()
{
}

BEGIN_MESSAGE_MAP(CmdReceiveEdit, CEdit)
	ON_WM_SETTEXT()
	ON_MESSAGE(WM_APP+1, OnUserMessagePasteOnly)
	ON_MESSAGE(WM_APP+2, OnUserMessageSetPos)
END_MESSAGE_MAP()

int CmdReceiveEdit::OnSetText(LPCTSTR text)
{
	if (mIsPasteOnly) {
		// テキスト設定のみ
		GetParent()->SendMessage(WM_APP + 11, 0, (LPARAM)text);
	}
	else {
		// コマンド実行
		GetParent()->SendMessage(WM_APP + 3, 0, (LPARAM)text);
	}

	mIsPasteOnly = false;
	return 0;
}

LRESULT CmdReceiveEdit::OnUserMessagePasteOnly(WPARAM wp, LPARAM lp)
{
	UNREFERENCED_PARAMETER(wp);
	UNREFERENCED_PARAMETER(lp);

	// これがよばれたら次回のWM_SETTEXTではコマンド入力だけとする
	mIsPasteOnly = true;
	return 0;
}

LRESULT CmdReceiveEdit::OnUserMessageSetPos(WPARAM wp, LPARAM lp)
{
	GetParent()->SendMessage(WM_APP + 12, wp, lp);
	return 0;
}

