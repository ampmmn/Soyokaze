#include "stdafx.h"
#include "gtest/gtest.h"
#include "control/CommandListCtrl.h"
#include "commands/core/CommandIF.h"

namespace {
	class TestCommand : public launcherapp::core::Command
	{
	public:
		TestCommand(const CString& name, const CString& description, const CString& type, bool& isDestroyed) :
			mName(name),
			mDescription(description),
			mType(type),
			mIsDestroyed(isDestroyed)
		{
		}

		~TestCommand()
		{
			mIsDestroyed = true;
		}

		CString GetName() override { return mName; }
		CString GetDescription() override { return mDescription; }
		CString GetTypeDisplayName() override { return mType; }
		bool CanExecute(String*) override { return false; }
		bool CanResolve() override { return false; }
		bool Resolve(CString&) override { return false; }
		bool IsAcceptArguments() override { return false; }
		bool GetAction(const HOTKEY_ATTR&, Action**) override { return false; }
		HICON GetIcon() override { return nullptr; }
		int Match(Pattern*) override { return Pattern::Mismatch; }
		bool IsAllowAutoExecute() override { return false; }
		bool GetHotKeyAttribute(CommandHotKeyAttribute&) override { return false; }
		Command* Clone() override { return nullptr; }
		bool Save(CommandEntryIF*) override { return false; }
		bool Load(CommandEntryIF*) override { return false; }
		bool QueryInterface(const launcherapp::core::IFID&, void**) override { return false; }

		uint32_t AddRef() override
		{
			return ++mRefCount;
		}

		uint32_t Release() override
		{
			uint32_t refCount = --mRefCount;
			if (refCount == 0) {
				delete this;
			}
			return refCount;
		}

	private:
		CString mName;
		CString mDescription;
		CString mType;
		bool& mIsDestroyed;
		uint32_t mRefCount{1};
	};
}

TEST(CommandListCtrlTest, FiltersCommandsAndKeepsThemAliveUntilTheListIsDestroyed)
{
	bool isFirstDestroyed = false;
	bool isSecondDestroyed = false;
	auto first = new TestCommand(_T("Alpha"), _T("green item"), _T("type A"), isFirstDestroyed);
	auto second = new TestCommand(_T("Beta"), _T("yellow item"), _T("type B"), isSecondDestroyed);
	std::vector<launcherapp::core::Command*> commands{first, second};

	{
		CommandListCtrl list;
		list.SetCommands(commands);
		first->Release();
		second->Release();

		EXPECT_EQ(2, list.GetVisibleCommandCount());
		EXPECT_TRUE(list.GetCommandAt(0)->GetName() == _T("Alpha"));

		list.SetFilterText(_T("yellow"));
		ASSERT_EQ(1, list.GetVisibleCommandCount());
		EXPECT_TRUE(list.GetCommandAt(0)->GetName() == _T("Beta"));
		EXPECT_FALSE(isFirstDestroyed);
		EXPECT_FALSE(isSecondDestroyed);

		list.SetFilterText(_T(""));
		EXPECT_EQ(2, list.GetVisibleCommandCount());
	}

	EXPECT_TRUE(isFirstDestroyed);
	EXPECT_TRUE(isSecondDestroyed);
}

TEST(CommandListCtrlTest, MatchesCommandNameDescriptionAndType)
{
	bool isDestroyed = false;
	auto command = new TestCommand(_T("Alpha"), _T("green item"), _T("type A"), isDestroyed);
	std::vector<launcherapp::core::Command*> commands{command};

	{
		CommandListCtrl list;
		list.SetCommands(commands);
		command->Release();

		list.SetFilterText(_T("Alpha"));
		EXPECT_EQ(1, list.GetVisibleCommandCount());
		list.SetFilterText(_T("green"));
		EXPECT_EQ(1, list.GetVisibleCommandCount());
		list.SetFilterText(_T("type A"));
		EXPECT_EQ(1, list.GetVisibleCommandCount());
		list.SetFilterText(_T("missing"));
		EXPECT_EQ(0, list.GetVisibleCommandCount());
	}

	EXPECT_TRUE(isDestroyed);
}
