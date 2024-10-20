#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include "commands/url_directoryindex/DirectoryIndexQueryResult.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

class CommandParam;

class URLDirectoryIndexCommand : 
	virtual public launcherapp::commands::common::UserCommandBase,
 	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	URLDirectoryIndexCommand();
	virtual ~URLDirectoryIndexCommand();

	void SetSubPath(const CString& subPath);
	CString GetSubPath();
	void LoadCanidates(bool& isHTML);

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	BOOL Execute(Parameter* param) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	bool IsPriorityRankEnabled() override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// Editable
	// コマンドを編集するためのダイアログを作成/取得する
	virtual bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	virtual bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	virtual bool CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmd) override;

// ExtraCandidateSource
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) override;
	void ClearCache() override;

	static CString GetType();

	static bool NewDialog(Parameter* param, URLDirectoryIndexCommand** newCmd);
	
public:
	URLDirectoryIndexCommand& SetParam(const CommandParam& param);
	URLDirectoryIndexCommand& GetParam(CommandParam& param);


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

