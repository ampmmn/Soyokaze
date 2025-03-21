#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/AppProfile.h"
#include "utility/IniFile.h"
#include <atlstr.h>
#include <regex>

class AppProfileTest : public ::testing::Test {
protected:
    CAppProfile* appProfile;

    void SetUp() override {
        appProfile = CAppProfile::Get();
    }

    void TearDown() override {
        // 必要に応じてクリーンアップコードを追加
    }
};


TEST_F(AppProfileTest, GetDirPath)
{
	TCHAR path[1024];
	CAppProfile::GetDirPath(path, 1024, false);

	std::wregex re(LR"(^.:\\Users\\.+?\\\.soyokaze)");

	EXPECT_TRUE(std::regex_match(path, re));
}

TEST_F(AppProfileTest, GetDirPath2)
{
	TCHAR path[1024];
	CAppProfile::GetDirPath(path, 1024, true);

	std::wregex re(LR"(^.:\\Users\\.+?\\\.soyokaze\\per_machine\\.+?)");

	EXPECT_TRUE(std::regex_match(path, re));
}


TEST_F(AppProfileTest, GetFilePath)
{
	TCHAR path[1024];
	CAppProfile::GetFilePath(path, 1024, false);

	std::wregex re(LR"(^.:\\Users\\.+?\\\.soyokaze\\settings.ini)");

	EXPECT_TRUE(std::regex_match(path, re));
}

TEST_F(AppProfileTest, InitializeProfileDir) {
    bool isNewCreated = false;
    EXPECT_TRUE(CAppProfile::InitializeProfileDir(&isNewCreated));
}

TEST_F(AppProfileTest, GetInt) {
    appProfile->Write(_T("TestSection"), _T("TestIntKey"), 42);
    EXPECT_EQ(appProfile->Get(_T("TestSection"), _T("TestIntKey"), 0), 42);
}

TEST_F(AppProfileTest, GetDouble) {
    appProfile->Write(_T("TestSection"), _T("TestDoubleKey"), 3.14);
    EXPECT_DOUBLE_EQ(appProfile->Get(_T("TestSection"), _T("TestDoubleKey"), 0.0), 3.14);
}

TEST_F(AppProfileTest, GetString) {
    appProfile->Write(_T("TestSection"), _T("TestStringKey"), _T("Hello, World!"));
    EXPECT_STREQ(appProfile->Get(_T("TestSection"), _T("TestStringKey"), _T("")), _T("Hello, World!"));
}

TEST_F(AppProfileTest, GetBinary) {
    std::vector<uint8_t> data = { 1, 2, 3, 4, 5 };
    appProfile->Write(_T("TestSection"), _T("TestBinaryKey"), data.data(), data.size());

    std::vector<uint8_t> outData(data.size());
    size_t len = appProfile->Get(_T("TestSection"), _T("TestBinaryKey"), outData.data(), outData.size());
    EXPECT_EQ(len, data.size());
    EXPECT_EQ(outData, data);
}

TEST_F(AppProfileTest, GetBinaryAsString) {
    appProfile->WriteStringAsBinary(_T("TestSection"), _T("TestBinaryStringKey"), _T("TestString"));
    EXPECT_STREQ(appProfile->GetBinaryAsString(_T("TestSection"), _T("TestBinaryStringKey"), _T("")), _T("TestString"));
}

TEST_F(AppProfileTest, WriteInt) {
    appProfile->Write(_T("TestSection"), _T("TestIntKey"), 42);
    EXPECT_EQ(appProfile->Get(_T("TestSection"), _T("TestIntKey"), 0), 42);
}

TEST_F(AppProfileTest, WriteDouble) {
    appProfile->Write(_T("TestSection"), _T("TestDoubleKey"), 3.14);
    EXPECT_DOUBLE_EQ(appProfile->Get(_T("TestSection"), _T("TestDoubleKey"), 0.0), 3.14);
}

TEST_F(AppProfileTest, WriteString) {
    appProfile->Write(_T("TestSection"), _T("TestStringKey"), _T("Hello, World!"));
    EXPECT_STREQ(appProfile->Get(_T("TestSection"), _T("TestStringKey"), _T("")), _T("Hello, World!"));
}

TEST_F(AppProfileTest, WriteBinary) {
    std::vector<uint8_t> data = { 1, 2, 3, 4, 5 };
    appProfile->Write(_T("TestSection"), _T("TestBinaryKey"), data.data(), data.size());

    std::vector<uint8_t> outData(data.size());
    size_t len = appProfile->Get(_T("TestSection"), _T("TestBinaryKey"), outData.data(), outData.size());
    EXPECT_EQ(len, data.size());
    EXPECT_EQ(outData, data);
}

TEST_F(AppProfileTest, WriteStringAsBinary) {
    appProfile->WriteStringAsBinary(_T("TestSection"), _T("TestBinaryStringKey"), _T("TestString"));
    EXPECT_STREQ(appProfile->GetBinaryAsString(_T("TestSection"), _T("TestBinaryStringKey"), _T("")), _T("TestString"));
}

