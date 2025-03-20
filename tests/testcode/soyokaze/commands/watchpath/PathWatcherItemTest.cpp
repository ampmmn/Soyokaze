#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/watchpath/PathWatcherItem.h"

using namespace launcherapp::commands::watchpath;

class PathWatcherItemTest : public ::testing::Test {
protected:
    PathWatcherItem item;

    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(PathWatcherItemTest, BuildExcludeFilterRegex_Success) {
    item.mExcludeFilter = _T("*.txt,*.log");
    std::unique_ptr<tregex> reg;

    bool result = item.BuildExcludeFilterRegex(reg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(reg != nullptr);
    EXPECT_TRUE(std::regex_match(_T("test.txt"), *reg));
    EXPECT_TRUE(std::regex_match(_T("test.log"), *reg));
    EXPECT_FALSE(std::regex_match(_T("test.doc"), *reg));
}

TEST_F(PathWatcherItemTest, BuildExcludeFilterRegex_Success2) {
    item.mExcludeFilter = _T("[*.txt");
    std::unique_ptr<tregex> reg;

    bool result = item.BuildExcludeFilterRegex(reg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(reg != nullptr);
}

TEST_F(PathWatcherItemTest, BuildExcludeFilterRegex_Success3) {
    item.mExcludeFilter = _T("*.txt,~$*");
    std::unique_ptr<tregex> reg;

    bool result = item.BuildExcludeFilterRegex(reg);

    EXPECT_TRUE(result);
    EXPECT_TRUE(reg != nullptr);
}

TEST_F(PathWatcherItemTest, WildcardToRegexp) {
    CString input = _T("*.txt");
    tstring output;

    item.WildcardToRegexp(input, output);

    EXPECT_EQ(output, _T(".*\\.txt"));
}

TEST_F(PathWatcherItemTest, WildcardToRegexp2) {
    CString input = _T("~$*");
    tstring output;

    item.WildcardToRegexp(input, output);
        
    EXPECT_EQ(output, _T("~\\$.*"));
}

