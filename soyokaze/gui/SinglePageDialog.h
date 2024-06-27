// あ
#pragma once

#include "utility/TopMostMask.h"
#include "gui/DDXWrapper.h"

namespace launcherapp {
namespace gui {

class SinglePageDialog : public CDialogEx
{
public:
	SinglePageDialog(UINT resId, CWnd* parent = nullptr);
	virtual ~SinglePageDialog();

protected:
	void SetHelpPageId(const CString& id);
	virtual bool ShowHelp();
private:
	// ヘルプページを表す識別子
	CString mHelpPageId;

	TopMostMask mTopMostMask;
	HACCEL mAccel;
protected:
	virtual BOOL OnInitDialog() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint pt);
	afx_msg void OnCommandHelp();
};


}
}
