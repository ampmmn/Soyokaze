#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/core/CommandMap.h"

namespace {

class TestCommand : public launcherapp::core::Command
{
public:
	explicit TestCommand(const CString& name) : mName(name)
	{
	}

	bool QueryInterface(const launcherapp::core::IFID&, void**) override
	{
		return false;
	}

	uint32_t AddRef() override
	{
		return ++mRefCount;
	}

	uint32_t Release() override
	{
		return --mRefCount;
	}

	CString GetName() override
	{
		return mName;
	}

	void SetName(const CString& name)
	{
		mName = name;
	}

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
	bool GetHotKeyAttribute(CommandHotKeyAttribute&) override { return false; }
	launcherapp::core::Command* Clone() override { return nullptr; }
	bool Save(CommandEntryIF*) override { return false; }
	bool Load(CommandEntryIF*) override { return false; }

private:
	CString mName;
	uint32_t mRefCount{1};
};

}

TEST(CommandMapTest, KeepsSortedIterationAndSupportsRename)
{
	TestCommand alpha(_T("alpha"));
	TestCommand bravo(_T("bravo"));
	TestCommand charlie(_T("charlie"));
	CommandMap commandMap;
	commandMap.Register(&charlie);
	commandMap.Register(&alpha);
	commandMap.Register(&bravo);

	std::vector<launcherapp::core::Command*> commands;
	commandMap.Enumerate(commands);
	ASSERT_EQ(3u, commands.size());
	EXPECT_EQ(&alpha, commands[0]);
	EXPECT_EQ(&bravo, commands[1]);
	EXPECT_EQ(&charlie, commands[2]);
	for (auto command : commands) {
		command->Release();
	}

	alpha.SetName(_T("zulu"));
	EXPECT_TRUE(commandMap.Reregister(&alpha));
	EXPECT_FALSE(commandMap.Has(CString(_T("alpha"))));
	EXPECT_TRUE(commandMap.Has(CString(_T("zulu"))));

	commands.clear();
	commandMap.Enumerate(commands);
	ASSERT_EQ(3u, commands.size());
	EXPECT_EQ(&bravo, commands[0]);
	EXPECT_EQ(&charlie, commands[1]);
	EXPECT_EQ(&alpha, commands[2]);
	for (auto command : commands) {
		command->Release();
	}
}
