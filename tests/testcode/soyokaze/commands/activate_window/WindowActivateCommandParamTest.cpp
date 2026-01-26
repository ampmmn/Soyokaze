#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "commands/validation/CommandParamErrorCode.h"

// モックCommandEntryIF
class MockCommandEntry : public CommandEntryIF {
public:
    CString mName;
    std::map<CString, CString> strMap;
    std::map<CString, bool> boolMap;

    LPCTSTR GetName() override { return mName; }
    void MarkAsUsed() override {}
    bool IsUsedEntry() override { return false; }
    bool HasValue(LPCTSTR key) override { return strMap.count(key) > 0 || boolMap.count(key) > 0; }
    int GetValueType(LPCTSTR key) override { return 0; }
    int Get(LPCTSTR key, int defValue) override { return defValue; }
    void Set(LPCTSTR key, int value) override {}
    double Get(LPCTSTR key, double defValue) override { return defValue; }
    void Set(LPCTSTR key, double value) override {}
    CString Get(LPCTSTR key, LPCTSTR defValue) override {
        auto it = strMap.find(key);
        return it != strMap.end() ? it->second : (CString)defValue;
    }
    void Set(LPCTSTR key, const CString& value) override { strMap[key] = value; }
    bool Get(LPCTSTR key, bool defValue) override {
        auto it = boolMap.find(key);
        return it != boolMap.end() ? it->second : defValue;
    }
    void Set(LPCTSTR key, bool value) override { boolMap[key] = value; }
    size_t GetBytesLength(LPCTSTR key) override { return 0; }
    bool GetBytes(LPCTSTR key, uint8_t* buf, size_t bufLen) override { return false; }
    void SetBytes(LPCTSTR key, const uint8_t* buf, size_t bufLen) override {}
	bool DumpRawData(std::vector<uint8_t>& rawData) override { return false; }
};

using namespace launcherapp::commands::activate_window;
using namespace launcherapp::commands::validation;

TEST(WindowActivateCommandParam, NoName)
{
    CommandParam param;
    int errCode = -1;
    param.mName = _T("");
    EXPECT_FALSE(param.IsValid(_T(""), &errCode));
    EXPECT_EQ(CommandParamErrorCode::Common_NoName, errCode);
}



TEST(WindowActivateCommandParam, InvalidName)
{
    CommandParam param;
    int errCode = -1;
		// 名前に":"は使えない
    param.mName = _T("test:");
    EXPECT_FALSE(param.IsValid(_T("test"), &errCode));
    EXPECT_EQ(CommandParamErrorCode::Common_NameContainsIllegalChar, errCode);
}


TEST(WindowActivateCommandParam, DefultValidation)
{
    CommandParam param;
    int errCode = -1;
    param.mName = _T("test");
    param.mCaptionStr = _T("");
    param.mClassStr = _T("");
    EXPECT_FALSE(param.IsValid(_T("test"), &errCode));
    EXPECT_EQ(CommandParamErrorCode::ActivateWindow_CaptionAndClassBothEmpty, errCode);
}

TEST(WindowActivateCommandParam, ClassIsEmpty)
{
    CommandParam param;
    int errCode = -1;
    param.mName = _T("test");
    param.mCaptionStr = _T("a");
    param.mClassStr = _T("");
    EXPECT_TRUE(param.IsValid(_T("test"), &errCode));
    EXPECT_EQ(CommandParamErrorCode::Common_NoError, errCode);
}


TEST(WindowActivateCommandParam, InvalidCaptionRegExpPattern)
{
    CommandParam param;
    int errCode = -1;
    param.mName = _T("test");
    param.mCaptionStr = _T("?");
    param.mClassStr = _T("test");
		param.mIsUseRegExp = true;
    EXPECT_FALSE(param.IsValid(_T("test"), &errCode));
    EXPECT_EQ(CommandParamErrorCode::ActivateWindow_CaptionIsInvalid, errCode);
}

TEST(WindowActivateCommandParam, InvalidClassRegExpPattern)
{
    CommandParam param;
    int errCode = -1;
    param.mName = _T("test");
    param.mCaptionStr = _T("test");
    param.mClassStr = _T("?");
		param.mIsUseRegExp = true;
    EXPECT_FALSE(param.IsValid(_T("test"), &errCode));
    EXPECT_EQ(CommandParamErrorCode::ActivateWindow_ClassIsInvalid, errCode);
}

TEST(WindowActivateCommandParam, SaveAndLoad)
{
    CommandParam param;
    param.mName = _T("test");
    param.mDescription = _T("desc");
    param.mCaptionStr = _T("caption");
    param.mClassStr = _T("class");
    param.mIsUseRegExp = true;
    param.mIsNotifyIfWindowNotFound = true;
    param.mIsAllowAutoExecute = true;
    param.mIsHotKeyOnly = true;

    MockCommandEntry entry;
    param.Save(&entry);

    CommandParam loaded;
    loaded.Load(&entry);

    EXPECT_EQ(param.mDescription, loaded.mDescription);
    EXPECT_EQ(param.mCaptionStr, loaded.mCaptionStr);
    EXPECT_EQ(param.mClassStr, loaded.mClassStr);
    EXPECT_EQ(param.mIsUseRegExp, loaded.mIsUseRegExp);
    EXPECT_EQ(param.mIsNotifyIfWindowNotFound, loaded.mIsNotifyIfWindowNotFound);
    EXPECT_EQ(param.mIsAllowAutoExecute, loaded.mIsAllowAutoExecute);
    EXPECT_EQ(param.mIsHotKeyOnly, loaded.mIsHotKeyOnly);
}

TEST(WindowActivateCommandParam, BuildRegExpAndTryBuildRegExp)
{
    CommandParam param;
    param.mCaptionStr = _T(".*");
    param.mClassStr = _T(".*");
    param.mIsUseRegExp = true;
    CString errMsg;
    EXPECT_TRUE(param.BuildRegExp(&errMsg));
    EXPECT_TRUE(param.TryBuildRegExp(&errMsg));
}

TEST(WindowActivateCommandParam, BuildCaptionRegExp_Invalid)
{
    CommandParam param;
    param.mCaptionStr = _T("[invalid(");
    param.mIsUseRegExp = true;
    CString errMsg;
    EXPECT_FALSE(param.BuildCaptionRegExp(&errMsg));
    EXPECT_FALSE(errMsg.IsEmpty());
}

TEST(WindowActivateCommandParam, BuildClassRegExp_Invalid)
{
    CommandParam param;
    param.mClassStr = _T("[invalid(");
    param.mIsUseRegExp = true;
    CString errMsg;
    EXPECT_FALSE(param.BuildClassRegExp(&errMsg));
    EXPECT_FALSE(errMsg.IsEmpty());
}

TEST(WindowActivateCommandParam, IsMatchCaptionAndClass)
{
    CommandParam param;
    param.mCaptionStr = _T("abc");
    param.mClassStr = _T("xyz");
    param.mIsUseRegExp = false;
    EXPECT_TRUE(param.IsMatchCaption(_T("abc")));
    EXPECT_FALSE(param.IsMatchCaption(_T("def")));
    EXPECT_TRUE(param.IsMatchClass(_T("xyz")));
    EXPECT_FALSE(param.IsMatchClass(_T("uvw")));
}

TEST(WindowActivateCommandParam, IsUseRegExpAndHasRegExpr)
{
    CommandParam param;
    param.mCaptionStr = _T("abc");
    param.mClassStr = _T("");
    param.mIsUseRegExp = true;
    EXPECT_TRUE(param.IsUseRegExp());
    EXPECT_TRUE(param.HasCaptionRegExpr());
    EXPECT_FALSE(param.HasClassRegExpr());
}

TEST(WindowActivateCommandParam, IsNotifyIfWindowNotFound)
{
    CommandParam param;
    param.mIsNotifyIfWindowNotFound = true;
    EXPECT_TRUE(param.IsNotifyIfWindowNotFound());
    param.mIsNotifyIfWindowNotFound = false;
    EXPECT_FALSE(param.IsNotifyIfWindowNotFound());
}
