#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/AppProfile.h"
#include "utility/IniFile.h"
#include "utility/Path.h"
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


// テスト中に差し替えたUSERPROFILE(一時フォルダ)配下の .soyokaze のパスを取得する
static CString GetExpectedNormalDir()
{
	TCHAR buf[MAX_PATH_NTFS] = {};
	size_t len = 0;
	_tgetenv_s(&len, buf, _countof(buf), _T("USERPROFILE"));
	CString dir(buf);
	dir += _T("\\.soyokaze");
	return dir;
}

TEST_F(AppProfileTest, GetDirPath)
{
	TCHAR path[1024];
	CAppProfile::GetDirPath(path, 1024, false);

	EXPECT_STREQ((LPCTSTR)GetExpectedNormalDir(), path);
}

TEST_F(AppProfileTest, GetDirPath2)
{
	TCHAR path[1024];
	CAppProfile::GetDirPath(path, 1024, true);

	// PC別ディレクトリは .soyokaze\per_machine\<PC名> の下になる
	CString prefix = GetExpectedNormalDir() + _T("\\per_machine\\");
	EXPECT_EQ(0, _tcsnicmp(path, prefix, prefix.GetLength()));
}


TEST_F(AppProfileTest, GetFilePath)
{
	TCHAR path[1024];
	CAppProfile::GetFilePath(path, 1024, false);

	CString expected = GetExpectedNormalDir() + _T("\\settings.ini");
	EXPECT_STREQ((LPCTSTR)expected, path);
}

TEST_F(AppProfileTest, SetRunAsPortable)
{
	CAppProfile::SetRunAsPortable(true);

	TCHAR path[1024];
	CAppProfile::GetDirPath(path, 1024, false);
	Path portablePath(Path::MODULEFILEDIR, _T("profile"));
	EXPECT_STREQ(path, (LPCTSTR)portablePath);

	CAppProfile::SetRunAsPortable(false);
	CAppProfile::GetDirPath(path, 1024, false);
	EXPECT_STREQ((LPCTSTR)GetExpectedNormalDir(), path);
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

