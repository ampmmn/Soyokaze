#pragma once

#include <memory>

class MainWindowOptionButton : public CButton
{
public:
	MainWindowOptionButton();
	virtual ~MainWindowOptionButton();

public:
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	DECLARE_MESSAGE_MAP()

	afx_msg void OnMouseMove(UINT flags, CPoint pt);
	afx_msg void OnTimer(UINT_PTR timerId);
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

