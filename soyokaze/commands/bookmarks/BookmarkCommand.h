#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include "commands/bookmarks/Bookmarks.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace bookmarks {

class BookmarkCommand :
 	virtual public launcherapp::commands::common::UserCommandBase,
 	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	BookmarkCommand();
	virtual ~BookmarkCommand();

// Comand
	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(Parameter* param) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// ExtraCandidateSource
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) override;
	void ClearCache() override;

// Editable
	// コマンドを編集するためのダイアログを作成/取得する
	virtual bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	virtual bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	virtual bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	static CString GetType();

	static bool NewDialog(Parameter* param, std::unique_ptr<BookmarkCommand>& newCmd);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace bookmarks
} // end of namespace commands
} // end of namespace launcherapp

