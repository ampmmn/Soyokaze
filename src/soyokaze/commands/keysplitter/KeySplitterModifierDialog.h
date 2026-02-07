#pragma once

#include "control/SinglePageDialog.h"
#include <memory>
#include "commands/keysplitter/KeySplitterModifierState.h"
#include "commands/keysplitter/KeySplitterParam.h"

namespace launcherapp {
namespace commands {
namespace keysplitter {

class ModifierDialog : public launcherapp::control::SinglePageDialog
{
public:
	ModifierDialog(CWnd* parentWnd = nullptr);
	~ModifierDialog() override;

	void SetParam(const CommandParam& param);

	void SetItem(const ITEM& item);
	void GetItem(ITEM& item);
	void SetModifierState(const ModifierState& state);
	void GetModifierState(ModifierState& state) const;

protected:
	bool UpdateStatus();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	void OnUpdateStatus();

	ITEM mItem;
	const CommandParam* mParamPtr{nullptr};

	ModifierState mOrgState;
	ModifierState mState;
	CString mMessage;

// 実装
protected:
	void OnOK() override;
	DECLARE_MESSAGE_MAP()
	afx_msg void OnButtonBrowse();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};

} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

