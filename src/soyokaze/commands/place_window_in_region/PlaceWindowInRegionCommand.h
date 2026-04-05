#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace place_window_in_region {

class CommandParam;

class PlaceWindowInRegionCommand : 
	virtual public launcherapp::commands::common::UserCommandBase,
	virtual public launcherapp::commands::core::ExtraCandidateSource

{
public:
	PlaceWindowInRegionCommand();
	virtual ~PlaceWindowInRegionCommand();

	void SetParam(const CommandParam& param);

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

// Command
	CString GetName() override;
	CString GetDescription() override;
	CString GetTypeDisplayName() override;

	// 修飾キー押下状態に対応した実行アクションを取得する
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	int Match(Pattern* pattern) override;
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override;
	launcherapp::core::Command* Clone() override;

	bool Save(CommandEntryIF* entry) override;
	bool Load(CommandEntryIF* entry) override;

// Editable
	// コマンドを編集するためのダイアログを作成/取得する
	bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// ダイアログ上での編集結果をコマンドに適用する
	bool Apply(launcherapp::core::CommandEditor* editor) override;
	// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
	bool CreateNewInstanceFrom(launcherapp::core::CommandEditor*editor, Command** newCmd) override;

// ExtraCandidateSource
	bool QueryCandidates(Pattern* pattern, CommandQueryItemList& commands) override;
	void ClearCache() override;


	static CString GetType();
	static CString TypeDisplayName();

	static bool NewInstance(launcherapp::core::CommandEditor* editor, Command** newCmdPtr);
	static bool NewDialog(Parameter* param, PlaceWindowInRegionCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, PlaceWindowInRegionCommand** newCmdPtr);


protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace place_window_in_region
} // end of namespace commands
} // end of namespace launcherapp

