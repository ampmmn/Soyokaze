#include "pch.h"
#include "PreviewHostWindow.h"
#include "PreviewWindow.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace mainwindow {
namespace preview {

struct PreviewHostWindow::PImpl
{
	PreviewWindow* mPreviewWindow{nullptr};
	HWND mPreviousParent{nullptr};
	LONG_PTR mPreviousStyle{0};
	LONG_PTR mPreviousExStyle{0};
};

BEGIN_MESSAGE_MAP(PreviewHostWindow, CWnd)
	ON_WM_SIZE()
END_MESSAGE_MAP()

PreviewHostWindow::PreviewHostWindow() : in(std::make_unique<PImpl>())
{
}

PreviewHostWindow::~PreviewHostWindow()
{
	AttachPreviewWindow(nullptr);
	if (GetSafeHwnd()) {
		DestroyWindow();
	}
}

bool PreviewHostWindow::Create(CWnd* parent)
{
	if (GetSafeHwnd()) {
		return true;
	}
	if (parent == nullptr || parent->GetSafeHwnd() == nullptr) {
		return false;
	}

	LPCTSTR className = AfxRegisterWndClass(0, nullptr, (HBRUSH)(COLOR_WINDOW + 1));
	return CreateEx(0, className, _T("PreviewHostWindow"), WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
	                CRect(0, 0, 0, 0), parent, 0) != FALSE;
}

void PreviewHostWindow::AttachPreviewWindow(PreviewWindow* previewWindow)
{
	if (in->mPreviewWindow == previewWindow) {
		if (previewWindow) {
			HWND priorFocus = ::GetFocus();
			previewWindow->Enter();
			if (IsWindow(priorFocus) && ::GetFocus() != priorFocus) {
				::SetFocus(priorFocus);
			}
		}
		return;
	}

	PreviewWindow* priorWindow = in->mPreviewWindow;
	if (priorWindow) {
		HWND priorHwnd = priorWindow->GetHwnd();
		if (IsWindow(priorHwnd)) {
			::ShowWindow(priorHwnd, SW_HIDE);
			::SetParent(priorHwnd, in->mPreviousParent);
			::SetWindowLongPtr(priorHwnd, GWL_STYLE, in->mPreviousStyle);
			::SetWindowLongPtr(priorHwnd, GWL_EXSTYLE, in->mPreviousExStyle);
			::SetWindowPos(priorHwnd, nullptr, 0, 0, 0, 0,
			             SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_HIDEWINDOW);
		}
		in->mPreviewWindow = nullptr;
		in->mPreviousParent = nullptr;
		priorWindow->Leave();
	}

	if (previewWindow == nullptr || GetSafeHwnd() == nullptr) {
		return;
	}

	HWND priorFocus = ::GetFocus();
	HWND previewHwnd = previewWindow->GetHwnd();
	if (IsWindow(previewHwnd) == FALSE) {
		if (IsWindow(priorFocus)) {
			::SetFocus(priorFocus);
		}
		return;
	}

	in->mPreviousParent = ::GetParent(previewHwnd);
	in->mPreviousStyle = GetWindowLongPtr(previewHwnd, GWL_STYLE);
	in->mPreviousExStyle = GetWindowLongPtr(previewHwnd, GWL_EXSTYLE);
	::ShowWindow(previewHwnd, SW_HIDE);
	::SetParent(previewHwnd, GetSafeHwnd());
	LONG_PTR style = in->mPreviousStyle;
	style &= ~WS_POPUP;
	style |= WS_CHILD;
	::SetWindowLongPtr(previewHwnd, GWL_STYLE, style);
	::SetWindowPos(previewHwnd, nullptr, 0, 0, 0, 0,
	             SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED | SWP_HIDEWINDOW);

	in->mPreviewWindow = previewWindow;
	previewWindow->Enter();
	if (IsWindow(priorFocus) && ::GetFocus() != priorFocus) {
		::SetFocus(priorFocus);
	}
}

bool PreviewHostWindow::HasPreviewWindow() const
{
	if (in->mPreviewWindow == nullptr) {
		return false;
	}
	HWND hwnd = in->mPreviewWindow->GetHwnd();
	return IsWindow(hwnd) && (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_VISIBLE) != 0;
}

void PreviewHostWindow::ShowPreview(bool isVisible)
{
	if (GetSafeHwnd() == nullptr) {
		return;
	}
	bool shouldShow = isVisible;
	if (shouldShow && in->mPreviewWindow) {
		HWND hwnd = in->mPreviewWindow->GetHwnd();
		shouldShow = IsWindow(hwnd) && (GetWindowLongPtr(hwnd, GWL_STYLE) & WS_VISIBLE) != 0;
	}
	ShowWindow(shouldShow ? SW_SHOWNOACTIVATE : SW_HIDE);
}

void PreviewHostWindow::OnSize(UINT type, int cx, int cy)
{
	__super::OnSize(type, cx, cy);
	if (in->mPreviewWindow == nullptr) {
		return;
	}

	HWND hwnd = in->mPreviewWindow->GetHwnd();
	if (IsWindow(hwnd)) {
		::SetWindowPos(hwnd, nullptr, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

}
}
}
