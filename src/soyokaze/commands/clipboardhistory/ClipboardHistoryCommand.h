#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/SelectionBehavior.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

class ClipboardHistoryCommand :
	virtual	public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::core::SelectionBehavior
{
public:
	ClipboardHistoryCommand(const CString& prefix, uint64_t appendData, const CString& data);
	virtual ~ClipboardHistoryCommand();

	CString GetName() override;
	CString GetTypeDisplayName() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// SelectionBehavior
	// 選択された
	void OnSelect(Command* prior) override;
	// 選択解除された
	void OnUnselect(Command* next) override;
	// 実行後のウインドウを閉じる方法を決定する
	CloseWindowPolicy GetCloseWindowPolicy(uint32_t modifierMask) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(ClipboardHistoryCommand)

public:
	static CString TypeDisplayName();
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp

