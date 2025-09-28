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
	// $B%3%^%s%I$rJT=8$9$k$?$a$N%@%$%"%m%0$r:n@.(B/$B<hF@$9$k(B
	virtual bool CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor) override;
	// $B%@%$%"%m%0>e$G$NJT=87k2L$r%3%^%s%I$KE,MQ$9$k(B
	virtual bool Apply(launcherapp::core::CommandEditor* editor) override;
	// $B%@%$%"%m%0>e$G$NJT=87k2L$K4p$E$-!"?7$7$$%3%^%s%I$r:n@.(B($BJ#@=(B)$B$9$k(B
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

