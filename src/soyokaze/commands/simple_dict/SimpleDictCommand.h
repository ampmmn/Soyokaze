#pragma once

#include "commands/common/UserCommandBase.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include "commands/core/CommandQueryItem.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include <memory>

class CommandFile;

namespace launcherapp {
namespace commands {
namespace simple_dict {

class Dictionary;

class CommandUpdateListenerIF;

class SimpleDictCommand :
 	virtual public launcherapp::commands::common::UserCommandBase,
 	virtual public launcherapp::commands::core::ExtraCandidateSource
{
public:
	SimpleDictCommand();
	virtual ~SimpleDictCommand();

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	void UpdateDictionary(Dictionary& dict);

	CString GetName() override;
	CString GetDescription() override;
	CString GetGuideString() override;
	CString GetTypeDisplayName() override;

	bool CanExecute(String*) override;
	bool GetAction(uint32_t modifierFlags, Action** action) override;
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

	static CString GetType();

	static bool NewDialog(Parameter* param, SimpleDictCommand** newCmd);
	static bool LoadFrom(CommandFile* cmdFile, void* entry, SimpleDictCommand** newCmdPtr);

	void SetParam(const SimpleDictParam& param);
	const SimpleDictParam& GetParam();

	static CString TypeDisplayName();

private:
	void ReserveUpdate(bool isRegisterWather = true);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


} // end of namespace simple_dict
} // end of namespace commands
} // end of namespace launcherapp

