#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/NamedCommandHotKeyHandler.h"
#include "setting/AppPreference.h"

namespace {

class TestCommand : public launcherapp::core::Command
{
public:
	TestCommand(const CString& name, const CommandHotKeyAttribute& hotKeyAttr, bool hasHotKey) :
		mName(name),
		mHotKeyAttr(hotKeyAttr),
		mHasHotKey(hasHotKey)
	{
	}

	bool QueryInterface(const launcherapp::core::IFID&, void**) override { return false; }
	uint32_t AddRef() override { return ++mRefCount; }
	uint32_t Release() override { return --mRefCount; }
	CString GetName() override { return mName; }
	CString GetDescription() override { return _T(""); }
	CString GetTypeDisplayName() override { return _T(""); }
	bool CanExecute(String*) override { return false; }
	bool CanResolve() override { return false; }
	bool Resolve(CString&) override { return false; }
	bool IsAcceptArguments() override { return false; }
	bool GetAction(const HOTKEY_ATTR&, launcherapp::actions::core::Action**) override { return false; }
	HICON GetIcon() override { return nullptr; }
	int Match(Pattern*) override { return Pattern::Mismatch; }
	bool IsAllowAutoExecute() override { return false; }
	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override
	{
		attr = mHotKeyAttr;
		return mHasHotKey;
	}
	launcherapp::core::Command* Clone() override { return nullptr; }
	bool Save(CommandEntryIF*) override { return false; }
	bool Load(CommandEntryIF*) override { return false; }

private:
	CString mName;
	CommandHotKeyAttribute mHotKeyAttr;
	bool mHasHotKey;
	uint32_t mRefCount{1};
};

class TestCommandRepositoryListener : public launcherapp::core::CommandRepositoryListenerIF
{
public:
	void OnBeforeLoad() override {}
	void OnNewCommand(launcherapp::core::Command* command) override { mAddedCommands.push_back(command); }
	void OnDeleteCommand(launcherapp::core::Command* command) override { mDeletedCommands.push_back(command); }
	void OnPatternReloaded() override { ++mPatternReloadCount; }

	std::vector<launcherapp::core::Command*> mAddedCommands;
	std::vector<launcherapp::core::Command*> mDeletedCommands;
	int mPatternReloadCount{0};
};

bool FindHotKeyMapping(
	const CommandHotKeyMappings& mappings,
	const CString& name,
	CommandHotKeyAttribute& attr
)
{
	for (int index = 0; index < mappings.GetItemCount(); ++index) {
		if (mappings.GetName(index) != name) {
			continue;
		}
		mappings.GetHotKeyAttr(index, attr);
		return true;
	}
	return false;
}

}

TEST(CommandRepositoryTest, RegistersCommandsAndReplacesThemInOneBatch)
{
	auto repository = launcherapp::core::CommandRepository::GetInstance();
	static LONG testNumber = 0;
	CString suffix;
	suffix.Format(_T("%ld"), InterlockedIncrement(&testNumber));
	CString commandName(_T("__CommandRepositoryTest_"));
	commandName += suffix;
	CString otherCommandName = commandName + _T("_other");
	ASSERT_FALSE(repository->HasCommand(commandName));
	ASSERT_FALSE(repository->HasCommand(otherCommandName));

	CommandHotKeyAttribute previousHotKey(MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_F24);
	CommandHotKeyAttribute importedHotKey(MOD_CONTROL | MOD_ALT | MOD_SHIFT, VK_F23);
	TestCommand existingCommand(commandName, previousHotKey, true);
	TestCommand otherCommand(otherCommandName, CommandHotKeyAttribute(), false);
	TestCommand replacementCommand(commandName, importedHotKey, true);

	auto preference = AppPreference::Get();
	CommandHotKeyMappings previousPreferenceMappings;
	preference->GetCommandKeyMappings(previousPreferenceMappings);

	int hotKeyOwner = 0;
	RefPtr<NamedCommandHotKeyHandler> hotKeyHandler(new NamedCommandHotKeyHandler(commandName), false);
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	ASSERT_TRUE(hotKeyManager->Register(&hotKeyOwner, hotKeyHandler.get(), previousHotKey));

	TestCommandRepositoryListener listener;
	repository->RegisterListener(&listener);
	existingCommand.AddRef();
	otherCommand.AddRef();
	repository->RegisterCommands({&existingCommand, &otherCommand});

	EXPECT_TRUE(repository->HasCommand(commandName));
	EXPECT_TRUE(repository->HasCommand(otherCommandName));
	EXPECT_EQ(2u, listener.mAddedCommands.size());
	EXPECT_TRUE(listener.mDeletedCommands.empty());

	replacementCommand.AddRef();
	repository->RegisterCommands({&replacementCommand}, {&existingCommand});

	EXPECT_TRUE(repository->HasCommand(commandName));
	EXPECT_TRUE(repository->HasCommand(otherCommandName));
	EXPECT_EQ(3u, listener.mAddedCommands.size());
	EXPECT_EQ(1u, listener.mDeletedCommands.size());
	if (listener.mDeletedCommands.empty() == false) {
		EXPECT_EQ(&existingCommand, listener.mDeletedCommands[0]);
	}

	CommandHotKeyMappings currentPreferenceMappings;
	preference->GetCommandKeyMappings(currentPreferenceMappings);
	CommandHotKeyAttribute actualHotKey;
	if (FindHotKeyMapping(currentPreferenceMappings, commandName, actualHotKey)) {
		EXPECT_EQ(previousHotKey, actualHotKey);
	}
	else {
		ADD_FAILURE() << "置き換え後も既存のホットキー設定が保持されること";
	}

	std::vector<launcherapp::core::Command*> registeredCommands;
	repository->EnumCommands(registeredCommands);
	bool foundReplacement = false;
	for (auto command : registeredCommands) {
		if (command->GetName() == commandName) {
			foundReplacement = command == &replacementCommand;
		}
		command->Release();
	}
	EXPECT_TRUE(foundReplacement);

	repository->RegisterCommands({}, {&replacementCommand, &otherCommand});
	repository->UnregisterListener(&listener);
	hotKeyManager->Clear(&hotKeyOwner);
	preference->SetCommandKeyMappings(previousPreferenceMappings);
}
