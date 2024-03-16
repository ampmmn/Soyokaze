// あ
#include "pch.h"
#include "CaptureIconLabel.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CaptureIconLabel::CaptureIconLabel() :
	mCursorHandle(nullptr),
	mOldCursorHandle(nullptr)
{
}

CaptureIconLabel::~CaptureIconLabel()
{
	if (mCursorHandle) {
		//DestroyCursor(mCursorHandle);
	}
}

BEGIN_MESSAGE_MAP(CaptureIconLabel, IconLabel)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CaptureIconLabel::OnLButtonDown(UINT, CPoint)
{
	if (mCursorHandle == nullptr) {
		mCursorHandle = (HCURSOR)LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDC_CAPTUREWINDOW), IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE);
	}

	SetCapture();
	mOldCursorHandle = ::SetCursor(mCursorHandle);
}

void CaptureIconLabel::OnLButtonUp(UINT, CPoint point)
{
	ReleaseCapture();
	if (mOldCursorHandle) {
		::SetCursor(mOldCursorHandle);
		mOldCursorHandle = nullptr;
	}

	ClientToScreen(&point);
	CWnd* wnd = WindowFromPoint(point);
	if (wnd == nullptr) {
		AfxMessageBox(_T("failed to get wnd"));
		return;
	}

	// キャプチャ対象のウインドウハンドルを親に通知
	GetParent()->PostMessage(WM_APP+6, 0, (LPARAM)wnd->GetSafeHwnd());

}

