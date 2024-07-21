#include "stdafx.h"
#include "gtest/gtest.h"
#include "mainwindow/CandidateList.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace CandidateListTest_local {

struct DummyCommand : public launcherapp::core::Command
{
	DummyCommand() : mDesc(_T("dummy description"))
	{
	}
	DummyCommand(LPCTSTR p) : mDesc(p)
	{
	}

	CString GetName() override
	{
		return _T("Dummy name");
	}

	CString GetDescription() override
	{
		return mDesc;
	}

	CString GetTypeDisplayName() override
	{
		return _T("");
	}

	CString GetGuideString() override
	{
		return _T("");
	}

	BOOL Execute(const Parameter& param) override
	{
		return TRUE;
	}

	CString GetErrorString() override
	{
		return _T("");
	}

	HICON GetIcon() override
	{
		return nullptr;
	}

	int Match(Pattern* pattern) override
	{
		return Pattern::WholeMatch;
	}

	bool IsEditable() override
	{
		return false;
	}
	bool IsDeletable() override
	{
		return false;
	}

	int EditDialog(HWND parent) override
	{
		return 0;
	}

	bool GetHotKeyAttribute(CommandHotKeyAttribute& attr) override
	{
		return false;
	}


	bool IsPriorityRankEnabled() override
	{
		return true;
	}

	Command* Clone() override
	{
		return new DummyCommand();
	}

	bool Save(CommandEntryIF* entry) override
	{
		return false;
	}

	bool Load(CommandEntryIF* entry) override
	{
		return false;
	}

	uint32_t AddRef() override
	{
		return 1;
	}

	uint32_t Release() override
	{
		delete this;
		return 0;
	}

	CString mDesc;
};

struct DummyListener : public CandidateListListenerIF
{
	void OnUpdateSelect(void* sender) override
	{
		mIsOnUpdateSelectCalled = true;
	}
	void OnUpdateItems(void* sender) override
	{
		mIsOnUpdateItemsCalled = true;
	}

	bool mIsOnUpdateSelectCalled = false;
	bool mIsOnUpdateItemsCalled = false;
};

}

using namespace CandidateListTest_local;

TEST(CandidateList, CanConstructAndDestruct)
{
	CandidateList candidates;
	EXPECT_EQ(0, candidates.GetSize());
	EXPECT_TRUE(candidates.IsEmpty());
	EXPECT_EQ(-1, candidates.GetCurrentSelect());
	EXPECT_TRUE(candidates.GetCommand(0) == nullptr);
	EXPECT_TRUE(candidates.GetCurrentCommand() == nullptr);
}

TEST(CandidateList, CanSetItems)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);

	// SetItemsでセットすると所有権が移るのでサイズは0になる
	EXPECT_EQ(0, commands.size());

	// オブジェクトの要素数は1になる
	EXPECT_EQ(1, candidates.GetSize());
	EXPECT_FALSE(candidates.IsEmpty());
	EXPECT_EQ(0, candidates.GetCurrentSelect());
	EXPECT_TRUE(candidates.GetCommand(0) != nullptr);

	// 
}

TEST(CandidateList, GetCommandTest)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);

	EXPECT_TRUE(candidates.GetCommand(-1) == nullptr);
	EXPECT_TRUE(candidates.GetCommand(1) == nullptr);
	EXPECT_TRUE(candidates.GetCommand(0) != nullptr);
}



TEST(CandidateList, CanClear)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);

	candidates.Clear();
	EXPECT_EQ(0, candidates.GetSize());
}

TEST(CandidateList, GetCurrentCommandDescritpionTest)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());
	commands.push_back(new DummyCommand(_T("")));

	CandidateList candidates;
	candidates.SetItems(commands);

	EXPECT_EQ(CString(_T("dummy description")), candidates.GetCurrentCommandDescription());

	EXPECT_TRUE(candidates.SetCurrentSelect(1));
	EXPECT_EQ(CString(_T("Dummy name")), candidates.GetCurrentCommandDescription());

	EXPECT_TRUE(candidates.SetCurrentSelect(1));
}

TEST(CandidateList, GetCurrentCommandDescritpionTest2)
{
	CandidateList candidates;

	EXPECT_EQ(CString(_T("")), candidates.GetCurrentCommandDescription());

}

TEST(CandidateList, OffsetCurrentSelect)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);

	EXPECT_TRUE(candidates.OffsetCurrentSelect(0, true));

	EXPECT_EQ(0, candidates.GetCurrentSelect());
	EXPECT_TRUE(candidates.OffsetCurrentSelect(2, false));
	EXPECT_EQ(1, candidates.GetCurrentSelect());
	EXPECT_TRUE(candidates.OffsetCurrentSelect(-2, false));
	EXPECT_EQ(0, candidates.GetCurrentSelect());
}

TEST(CandidateList, AddListenerTest1)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());

	CandidateList candidates;

	DummyListener listener;
	candidates.AddListener(&listener);

	candidates.SetItems(commands);
	EXPECT_FALSE(listener.mIsOnUpdateSelectCalled);
	EXPECT_TRUE(listener.mIsOnUpdateItemsCalled);

	candidates.RemoveListener(&listener);
}

TEST(CandidateList, AddListenerTest2)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);

	DummyListener listener;
	candidates.AddListener(&listener);

	EXPECT_TRUE(candidates.SetCurrentSelect(1));

	EXPECT_TRUE(listener.mIsOnUpdateSelectCalled);
	EXPECT_FALSE(listener.mIsOnUpdateItemsCalled);

	candidates.RemoveListener(&listener);
}

TEST(CandidateList, AddListenerTest3)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);

	DummyListener listener;
	candidates.AddListener(&listener);

	EXPECT_TRUE(candidates.OffsetCurrentSelect(-1, true));
	EXPECT_EQ(1, candidates.GetCurrentSelect());

	EXPECT_TRUE(listener.mIsOnUpdateSelectCalled);
	EXPECT_FALSE(listener.mIsOnUpdateItemsCalled);

	candidates.RemoveListener(&listener);
}

TEST(CandidateList, AddListenerTest4)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);
	candidates.SetCurrentSelect(1);

	DummyListener listener;
	candidates.AddListener(&listener);

	EXPECT_TRUE(candidates.OffsetCurrentSelect(1, true));
	EXPECT_EQ(0, candidates.GetCurrentSelect());

	EXPECT_TRUE(listener.mIsOnUpdateSelectCalled);
	EXPECT_FALSE(listener.mIsOnUpdateItemsCalled);

	candidates.RemoveListener(&listener);
}

TEST(CandidateList, AddListenerTest5)
{
	std::vector<launcherapp::core::Command*> commands;
	commands.push_back(new DummyCommand());
	commands.push_back(new DummyCommand());

	CandidateList candidates;
	candidates.SetItems(commands);
	candidates.SetCurrentSelect(1);

	DummyListener listener;
	candidates.AddListener(&listener);

	candidates.Clear();

	EXPECT_FALSE(listener.mIsOnUpdateSelectCalled);
	EXPECT_TRUE(listener.mIsOnUpdateItemsCalled);

	candidates.RemoveListener(&listener);
}

