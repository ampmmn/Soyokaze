#include "pch.h"
#include "framework.h"
#include "CmdReceiveEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CmdReceiveEdit::CmdReceiveEdit(CWnd* pParent)
{
}

CmdReceiveEdit::~CmdReceiveEdit()
{
}

BEGIN_MESSAGE_MAP(CmdReceiveEdit, CEdit)
	ON_WM_SETTEXT()
END_MESSAGE_MAP()

int CmdReceiveEdit::OnSetText(LPCTSTR text)
{
	GetParent()->SendMessage(WM_APP + 3, 0, (LPARAM)text);
	return 0;
}
