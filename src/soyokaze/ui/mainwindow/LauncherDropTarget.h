#pragma once

class LauncherDropTarget : public COleDropTarget
{
public:
	LauncherDropTarget(CWnd* parent);
	virtual ~LauncherDropTarget();

	CWnd* mParent;
	UINT mUrlFomatId;

	DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point) override;
	DROPEFFECT OnDragOver(CWnd* wnd, COleDataObject* dataObj, DWORD keyState, CPoint point) override;

	BOOL OnDrop(CWnd* wnd, COleDataObject* dataObj, DROPEFFECT dropEffect, CPoint point) override;
	void OnDragLeave(CWnd* pWnd) override;
};

