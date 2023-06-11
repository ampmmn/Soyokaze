
// SoyokazeDlg.h : ヘッダー ファイル
//

#pragma once

#include <vector>
#include <memory>
#include "gui/KeywordEdit.h"
#include "gui/CmdReceiveEdit.h"
#include "gui/CaptureIconLabel.h"
#include "gui/SoyokazeDropTarget.h"

namespace soyokaze {
namespace core {
	class AppHotKey;
	class Command;
	class CommandHotKeyManager;
	class CommandRepository;
}
}

class SharedHwnd;
class ExecHistory;
class WindowPosition;
class WindowTransparency;


namespace soyokaze {
namespace commands {
namespace filter {


class FilterDialog : public CDialogEx
{
	using CommandRepository = soyokaze::core::CommandRepository;
	using AppHotKey = soyokaze::core::AppHotKey;

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
	DECLARE_MESSAGE_MAP()
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace soyokaze

