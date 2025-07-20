#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/core/CommandQueryItemList.h"
#include "helper/DummyCommand.h"

using namespace launcherapp;
using Command = launcherapp::core::Command;

TEST(CommandQueryItemList, DefaultConstructor)
{
	CommandQueryItemList itemList;
	EXPECT_EQ(0, itemList.GetItemCount());

	Command* cmd = nullptr;
	EXPECT_FALSE(itemList.FindWholeMatchItem(&cmd));
}

TEST(CommandQueryItemList, AddAndGetItem)
{
    CommandQueryItemList itemList;
    DummyCommand* cmd1 = new DummyCommand(_T("Dummy"));
    DummyCommand* cmd2 = new DummyCommand();

    CommandQueryItem item1(Pattern::WholeMatch, cmd1);
    CommandQueryItem item2(Pattern::PartialMatch, cmd2);

    itemList.Add(item1);
    itemList.Add(item2);

    EXPECT_EQ(2, itemList.GetItemCount());

    CommandQueryItem getItem;
    EXPECT_TRUE(itemList.GetItem(0, &getItem));
    EXPECT_EQ(Pattern::WholeMatch, getItem.mMatchLevel);
    EXPECT_STREQ(_T("Dummy"), getItem.mCommand->GetDescription());

    EXPECT_TRUE(itemList.GetItem(1, &getItem));
    EXPECT_EQ(Pattern::PartialMatch, getItem.mMatchLevel);

    // 範囲外アクセス
    EXPECT_FALSE(itemList.GetItem(2, &getItem));
}

TEST(CommandQueryItemList, AddWeakMatchItem)
{
    CommandQueryItemList itemList;
    DummyCommand* cmd1 = new DummyCommand(_T("Dummy"));

    CommandQueryItem item1(Pattern::WeakMatch, cmd1);
    itemList.Add(item1);

    EXPECT_EQ(1, itemList.GetItemCount());
}

TEST(CommandQueryItemList, AddHiddenMatchItem)
{
    CommandQueryItemList itemList;
    DummyCommand* cmd1 = new DummyCommand(_T("Dummy"));

    CommandQueryItem item1(Pattern::HiddenMatch, cmd1);
    itemList.Add(item1);

		// HiddenMatchは検索結果には影響しない
    EXPECT_EQ(0, itemList.GetItemCount());
}


TEST(CommandQueryItemList, FindWholeMatchItem)
{
    DummyCommand* cmd1 = new DummyCommand();
    DummyCommand* cmd2 = new DummyCommand();

    CommandQueryItem item1(Pattern::WholeMatch, cmd1);
    CommandQueryItem item2(Pattern::PartialMatch, cmd2);

    Command* foundCmd = nullptr;
    {
        CommandQueryItemList itemList;
        itemList.Add(item1);
        itemList.Add(item2);
        
        EXPECT_TRUE(itemList.FindWholeMatchItem(&foundCmd));
    }
    EXPECT_EQ(foundCmd, cmd1);

	// itemListのスコープを抜けると、item1の参照カウントが減る    
	// そのため、cmd1の参照カウントは1になる
  
	EXPECT_EQ(1, foundCmd->Release());
}

TEST(CommandQueryItemList, FindWholeMatchItemWithNoWholeMatch)
{
    CommandQueryItemList itemList;
    DummyCommand* cmd1 = new DummyCommand(_T("Cmd1"));
    DummyCommand* cmd2 = new DummyCommand(_T("Cmd2"));
    DummyCommand* cmd3 = new DummyCommand(_T("Cmd3"));

    // すべて同じ一致レベルで追加
    CommandQueryItem item1(Pattern::PartialMatch, cmd1);
    CommandQueryItem item2(Pattern::PartialMatch, cmd2);
    CommandQueryItem item3(Pattern::PartialMatch, cmd3);

    itemList.Add(item1);
    itemList.Add(item2);
    itemList.Add(item3);

    Command* foundCmd = nullptr;
		EXPECT_FALSE(itemList.FindWholeMatchItem(&foundCmd));
}

TEST(CommandQueryItemList, Sort)
{
    CommandQueryItemList itemList;
    DummyCommand* cmd1 = new DummyCommand();
    DummyCommand* cmd2 = new DummyCommand();
    DummyCommand* cmd3 = new DummyCommand();

    // mMatchLevelが大きい順に並ぶと仮定
    CommandQueryItem item1(Pattern::WholeMatch, cmd1);
    CommandQueryItem item2(Pattern::PartialMatch, cmd2);
    CommandQueryItem item3(Pattern::FrontMatch, cmd3);

    itemList.Add(item1);
    itemList.Add(item2);
    itemList.Add(item3);

    itemList.Sort();

    CommandQueryItem getItem;
    EXPECT_TRUE(itemList.GetItem(0, &getItem));
    EXPECT_EQ(Pattern::WholeMatch, getItem.mMatchLevel);
    EXPECT_TRUE(itemList.GetItem(1, &getItem));
    EXPECT_EQ(Pattern::FrontMatch, getItem.mMatchLevel);
    EXPECT_TRUE(itemList.GetItem(2, &getItem));
    EXPECT_EQ(Pattern::PartialMatch, getItem.mMatchLevel);

}

TEST(CommandQueryItemList, SortWithSameMatchLevel)
{
    CommandQueryItemList itemList;
    DummyCommand* cmd1 = new DummyCommand(_T("Cmd1"));
    DummyCommand* cmd2 = new DummyCommand(_T("Cmd2"));
    DummyCommand* cmd3 = new DummyCommand(_T("Cmd3"));

    // すべて同じ一致レベルで追加
    CommandQueryItem item1(Pattern::PartialMatch, cmd1);
    CommandQueryItem item2(Pattern::PartialMatch, cmd2);
    CommandQueryItem item3(Pattern::PartialMatch, cmd3);

    itemList.Add(item1);
    itemList.Add(item2);
    itemList.Add(item3);

    itemList.Sort();

    // 並び順は追加順になることを期待
    CommandQueryItem getItem;
    EXPECT_TRUE(itemList.GetItem(0, &getItem));
    EXPECT_STREQ(_T("Cmd1"), getItem.mCommand->GetDescription());
    EXPECT_TRUE(itemList.GetItem(1, &getItem));
    EXPECT_STREQ(_T("Cmd2"), getItem.mCommand->GetDescription());
    EXPECT_TRUE(itemList.GetItem(2, &getItem));
    EXPECT_STREQ(_T("Cmd3"), getItem.mCommand->GetDescription());
}
