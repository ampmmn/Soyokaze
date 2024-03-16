#include "pch.h"
#include "SoyokazeDropTarget.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SoyokazeDropTarget::SoyokazeDropTarget(CWnd* parent) : mParent(parent)
{
	mUrlFomatId = RegisterClipboardFormat(CFSTR_INETURL);
}

SoyokazeDropTarget::~SoyokazeDropTarget()
{
}


DROPEFFECT SoyokazeDropTarget::OnDragEnter(
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

DROPEFFECT SoyokazeDropTarget::OnDragOver(
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
BOOL SoyokazeDropTarget::OnDrop(
	CWnd* wnd,
	COleDataObject* dataObj,
	DROPEFFECT dropEffect,
	CPoint point
)
{
	mParent->SendMessage(WM_APP+5, (WPARAM)dataObj, (LPARAM)wnd);

	return TRUE;
}

void SoyokazeDropTarget::OnDragLeave(CWnd* pWnd)
{
	__super::OnDragLeave(pWnd);
}
