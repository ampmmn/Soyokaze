#pragma once

#include "gui/SinglePageDialog.h"
#include <memory>
#include "commands/keysplitter/KeySplitterModifierState.h"

class ModalComboBox;

namespace launcherapp {
namespace commands {
namespace keysplitter {

class ModifierDialog : public launcherapp::gui::SinglePageDialog
{
public:
	ModifierDialog(CWnd* parentWnd = nullptr);
	~ModifierDialog() override;

	void SetParam(const ModifierState& state);
	void GetParam(ModifierState& state) const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual BOOL OnInitDialog();

	BOOL mIsPressShift;
	BOOL mIsPressCtrl;
	BOOL mIsPressAlt;
	BOOL mIsPressWin;

// 実装
protected:
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};

} // end of namespace keysplitter
} // end of namespace commands
} // end of namespace launcherapp

