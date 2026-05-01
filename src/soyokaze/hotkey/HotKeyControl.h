#pragma once

#include "hotkey/CommandHotKeyAttribute.h"
#include <memory>

class HotKeyControl : public CEdit
{
public:
	HotKeyControl(CWnd* pParent = nullptr);
	~HotKeyControl();

	void SetNotifyId(UINT notifyId);
	void UpdateContent(CString text);

	bool EditHotKey(const CString& name, CommandHotKeyAttribute& attr, CWnd* parentWnd);

protected:
	UINT GetNotifyId();

	virtual void SetText(const CString& text);
	virtual void SetCurBanner(const CString& bannerMsg);
	virtual void Notify(UINT id);

public:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnSetFocus(CWnd* oldWindow);
	afx_msg void OnLButtonDown(UINT flags, CPoint pt);
	afx_msg void OnKeyDown(UINT,UINT,UINT);
	afx_msg void OnSysKeyDown(UINT,UINT,UINT);
	afx_msg void OnChar(UINT,UINT,UINT);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;


};

