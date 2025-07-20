#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/core/CommandQueryItem.h"
#include "helper/DummyCommand.h"

using namespace launcherapp;

TEST(CommandQueryItem, DefaultConstructor)
{
	CommandQueryItem item;
	EXPECT_EQ(Pattern::Mismatch, item.mMatchLevel);
	EXPECT_TRUE(item.mCommand.get() == nullptr);
}

TEST(CommandQueryItem, SetCommand)
{
	auto cmd = new DummyCommand();
	EXPECT_EQ(2, cmd->AddRef());

	{
		// このコンストラクタはcmdに対する参照カウントのインクリメントは行わない
		CommandQueryItem item(Pattern::PartialMatch, cmd);
		EXPECT_EQ(Pattern::PartialMatch, item.mMatchLevel);
		EXPECT_TRUE(item.mCommand.get() != nullptr);
	}
	// したがって、ここでReleaseするとオブジェクトは解放される
	EXPECT_EQ(0, cmd->Release());
}

// コピーコンストラクタでコピーできること
TEST(CommandQueryItem, CopyConstructor)
{
	auto cmd = new DummyCommand();
	EXPECT_EQ(2, cmd->AddRef());
	CommandQueryItem itemSrc(Pattern::WholeMatch, cmd);

	{
		CommandQueryItem itemDst(itemSrc);
		EXPECT_EQ(Pattern::WholeMatch, itemDst.mMatchLevel);
		EXPECT_TRUE(itemDst.mCommand.get() != nullptr);
	}

	EXPECT_EQ(1, cmd->Release());
}

// 空の入れ物のコピーがてきること
TEST(CommandQueryItem, CopyConstructor2)
{
	CommandQueryItem itemSrc;

	CommandQueryItem itemDst(itemSrc);

	EXPECT_EQ(Pattern::Mismatch, itemDst.mMatchLevel);
	EXPECT_TRUE(itemDst.mCommand.get() == nullptr);
}

// operator = でコピーできること
TEST(CommandQueryItem, CopyOperator)
{
	auto cmd = new DummyCommand();
	EXPECT_EQ(2, cmd->AddRef());
	CommandQueryItem itemSrc(Pattern::WholeMatch, cmd);

	{
		CommandQueryItem itemDst;
		itemDst = itemSrc;
		EXPECT_EQ(Pattern::WholeMatch, itemDst.mMatchLevel);
		EXPECT_TRUE(itemDst.mCommand.get() != nullptr);
	}

	EXPECT_EQ(1, cmd->Release());
}

// operator = で上書きしたとき、以前のコマンドの参照カウントがデクリメントされること
TEST(CommandQueryItem, CopyOperator2)
{
	auto cmd = new DummyCommand();
	EXPECT_EQ(2, cmd->AddRef());

	{
		CommandQueryItem itemDst(Pattern::WholeMatch, cmd);
		CommandQueryItem itemSrc(Pattern::PartialMatch, new DummyCommand());

		// 上書きをする
		itemDst = itemSrc;

		// 上書き後の内容が想定通りであること
		EXPECT_EQ(Pattern::PartialMatch, itemDst.mMatchLevel);
		EXPECT_TRUE(itemDst.mCommand.get() != nullptr);
	}

	EXPECT_EQ(0, cmd->Release());
}
