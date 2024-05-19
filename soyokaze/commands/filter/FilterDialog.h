
// LauncherMainWindow.h : ヘッダー ファイル
//

#pragma once

#include "setting/AppPreferenceListenerIF.h"

#include <vector>
#include <memory>
#include "gui/KeywordEdit.h"
#include "icon/CaptureIconLabel.h"

namespace launcherapp {
namespace core {
	class Command;
	class CommandHotKeyManager;
	class CommandRepository;
}
}

class SharedHwnd;
class ExecHistory;
class WindowPosition;
class WindowTransparency;


namespace launcherapp {
namespace commands {
namespace filter {


class FilterDialog : public CDialogEx
{
	using CommandRepository = launcherapp::core::CommandRepository;

// コンストラクション
public:
	FilterDialog(CWnd* pParent = nullptr);	// 標準コンストラクター
	virtual ~FilterDialog();

	void SetCommandName(const CString& name);
	void SetText(const CString& text);
	CString GetFilteredText();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV サポート
	void UpdateStatus();

// 実装
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;

	// 生成された、メッセージ割り当て関数
	BOOL OnInitDialog() override;
	void OnOK() override;
	void OnCancel() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnEditCommandChanged();
	afx_msg void OnLbnSelChange();
	afx_msg void OnLbnDblClkCandidate();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg LRESULT OnKeywordEditNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCandidatesCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLvnItemChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnSize(UINT type, int cx, int cy);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

