#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "commands/core/ExtraCandidateIF.h"
#include "commands/url_directoryindex/URLDirectoryIndexCommand.h"
#include "commands/url_directoryindex/DirectoryIndexQueryResult.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

class CommandParam;

class DirectoryIndexAdhocCommand :
 	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::commands::core::ContextMenuSource,
	virtual public launcherapp::commands::core::ExtraCandidate
{
public:
	DirectoryIndexAdhocCommand(URLDirectoryIndexCommand* baseCmd, const QueryResult& result);
	virtual ~DirectoryIndexAdhocCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;
	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューの表示名を取得する
	bool GetMenuItemName(int index, LPCWSTR* displayNamePtr) override;
	// メニュー選択時の処理を実行する
	bool SelectMenuItem(int index, Parameter* param) override;

// ExtraCandidate
	CString GetSourceName() override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(DirectoryIndexAdhocCommand)

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};




} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp


