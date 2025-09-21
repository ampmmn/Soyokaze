#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include "commands/filter/FilterResult.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace filter {

class CommandParam;

class FilterCommand :
 	virtual public launcherapp::commands::common::UserCommandBase,
 	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	FilterCommand();
	virtual ~FilterCommand();

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	bool CanExecute() override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
	CString GetErrorString() override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
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

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	static CString GetType();
	static CString TypeDisplayName();

	static bool NewDialog(Parameter* param, FilterCommand** newCmd);
	
public:
	FilterCommand& SetParam(const CommandParam& param);
	FilterCommand& GetParam(CommandParam& param);


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace filter
} // end of namespace commands
} // end of namespace launcherapp

