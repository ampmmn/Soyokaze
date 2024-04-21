#include "pch.h"
#include "LauncherDropTarget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

LauncherDropTarget::LauncherDropTarget(CWnd* parent) : mParent(parent)
{
	mUrlFomatId = RegisterClipboardFormat(CFSTR_INETURL);
}

LauncherDropTarget::~LauncherDropTarget()
{
}


DROPEFFECT LauncherDropTarget::OnDragEnter(
	CWnd* wnd, COleDataObject* dataObj, DWORD dwKeyState, CPoint point)
{
	mParent->SendMessage(WM_APP+4, 0, (LPARAM)wnd);

	if (dataObj->IsDataAvailable(CF_HDROP)) {
		return DROPEFFECT_COPY;
	}
	else if (dataObj->IsDataAvailable(mUrlFomatId)) {
		return DROPEFFECT_LINK;
	}

	return DROPEFFECT_NONE;
}

DROPEFFECT LauncherDropTarget::OnDragOver(
	CWnd* wnd,
 	COleDataObject* dataObj,
 	DWORD keyState,
 	CPoint point
)
{
	mParent->SendMessage(WM_APP+4, 0, (LPARAM)wnd);

	if (dataObj->IsDataAvailable(CF_HDROP)) {
		return DROPEFFECT_COPY;
	}
	else if (dataObj->IsDataAvailable(mUrlFomatId)) {
		return DROPEFFECT_LINK;
	}

	return DROPEFFECT_NONE;
}

/**
 *
 */
BOOL LauncherDropTarget::OnDrop(
	CWnd* wnd,
	COleDataObject* dataObj,
	DROPEFFECT dropEffect,
	CPoint point
)
{
	mParent->SendMessage(WM_APP+5, (WPARAM)dataObj, (LPARAM)wnd);

	return TRUE;
}

void LauncherDropTarget::OnDragLeave(CWnd* pWnd)
{
	__super::OnDragLeave(pWnd);
}
