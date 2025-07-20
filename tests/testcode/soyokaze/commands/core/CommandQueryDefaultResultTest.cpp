#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/core/CommandQueryDefaultResult.h"
#include "helper/DummyCommand.h"

using namespace launcherapp;
using namespace launcherapp::commands::core;
using Command = launcherapp::core::Command;

TEST(CommandQueryDefaultResult, DefaultState)
{
    auto* result = CommandQueryDefaultResult::Create();
    EXPECT_TRUE(result->IsEmpty());
    EXPECT_EQ(0u, result->GetCount());
    result->Release();
}

TEST(CommandQueryDefaultResult, AddAndGet)
{
    auto* result = CommandQueryDefaultResult::Create();
    DummyCommand* cmd1 = new DummyCommand(_T("Cmd1"));
    DummyCommand* cmd2 = new DummyCommand(_T("Cmd2"));

    CommandQueryItem item1(1, cmd1);
    CommandQueryItem item2(2, cmd2);

    result->Add(item1);
    result->Add(item2);

    EXPECT_FALSE(result->IsEmpty());
    EXPECT_EQ(2u, result->GetCount());

    Command* outCmd = nullptr;
    int matchLevel = -1;
    EXPECT_TRUE(result->Get(0, &outCmd, &matchLevel));
    EXPECT_STREQ(_T("Cmd1"), outCmd->GetDescription());
    EXPECT_EQ(1, matchLevel);

    EXPECT_TRUE(result->Get(1, &outCmd, &matchLevel));
    EXPECT_STREQ(_T("Cmd2"), outCmd->GetDescription());
    EXPECT_EQ(2, matchLevel);

    // 範囲外アクセス
    EXPECT_FALSE(result->Get(2, &outCmd, &matchLevel));

    EXPECT_EQ(0, result->Release());
}

TEST(CommandQueryDefaultResult, GetItem)
{
    auto* result = CommandQueryDefaultResult::Create();
    DummyCommand* cmd1 = new DummyCommand(_T("Cmd1"));
    CommandQueryItem item1(Pattern::WholeMatch, cmd1);
    result->Add(item1);

    int matchLevel = -1;
    Command* outCmd = result->GetItem(0, &matchLevel);
    EXPECT_NE(nullptr, outCmd);
    EXPECT_STREQ(_T("Cmd1"), outCmd->GetDescription());
    EXPECT_EQ(Pattern::WholeMatch, matchLevel);

    // 範囲外
    EXPECT_EQ(nullptr, result->GetItem(1));

    result->Release();
}

TEST(CommandQueryDefaultResult, RefCount)
{
    auto* result = CommandQueryDefaultResult::Create();
    EXPECT_EQ(2u, result->AddRef());
    EXPECT_EQ(1u, result->Release());
    EXPECT_EQ(0u, result->Release()); // ここでdeleteされるはず
}
