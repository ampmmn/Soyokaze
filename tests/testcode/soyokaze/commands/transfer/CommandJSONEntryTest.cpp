#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/transfer/CommandJSONEntry.h"
#include <nlohmann/json.hpp>

using launcherapp::commands::transfer::CommandJSONEntry;
using json = nlohmann::json;

TEST(CommandJSONEntryTest, Init)
{
    CommandJSONEntry entry(_T("testCommand"));
    entry.Init();
}

TEST(CommandJSONEntryTest, GetName)
{
    CommandJSONEntry entry(_T("testCommand"));
    ASSERT_STREQ(_T("testCommand"), entry.GetName());
}

TEST(CommandJSONEntryTest, MarkAsUsedAndIsUsedEntry)
{
    CommandJSONEntry entry(_T("testCommand"));
    ASSERT_FALSE(entry.IsUsedEntry());
    entry.MarkAsUsed();
    ASSERT_TRUE(entry.IsUsedEntry());
}

TEST(CommandJSONEntryTest, SetAndGetInt)
{
    CommandJSONEntry entry(_T("testCommand"));
    entry.Set(_T("intKey"), 123);
    ASSERT_EQ(123, entry.Get(_T("intKey"), 0));
    ASSERT_EQ(0, entry.Get(_T("nonExistentKey"), 0)); // Default value
    ASSERT_EQ(CommandJSONEntry::VALUE_TYPE_INT, entry.GetValueType(_T("intKey")));
}

TEST(CommandJSONEntryTest, SetAndGetDouble)
{
    CommandJSONEntry entry(_T("testCommand"));
    entry.Set(_T("doubleKey"), 123.45);
    ASSERT_DOUBLE_EQ(123.45, entry.Get(_T("doubleKey"), 0.0));
    ASSERT_DOUBLE_EQ(0.0, entry.Get(_T("nonExistentKey"), 0.0)); // Default value
    ASSERT_EQ(CommandJSONEntry::VALUE_TYPE_DOUBLE, entry.GetValueType(_T("doubleKey")));
}

TEST(CommandJSONEntryTest, SetAndGetString)
{
    CommandJSONEntry entry(_T("testCommand"));
    entry.Set(_T("stringKey"), CString(_T("testString")));
    ASSERT_STREQ(_T("testString"), entry.Get(_T("stringKey"), _T("")).GetString());
    ASSERT_STREQ(_T("defaultValue"), entry.Get(_T("nonExistentKey"), _T("defaultValue")).GetString()); // Default value
    ASSERT_EQ(CommandJSONEntry::VALUE_TYPE_STRING, entry.GetValueType(_T("stringKey")));
}

TEST(CommandJSONEntryTest, SetAndGetBool)
{
    CommandJSONEntry entry(_T("testCommand"));
    entry.Set(_T("boolKey"), true);
    ASSERT_TRUE(entry.Get(_T("boolKey"), false));
    ASSERT_FALSE(entry.Get(_T("nonExistentKey"), false)); // Default value
    ASSERT_EQ(CommandJSONEntry::VALUE_TYPE_BOOL, entry.GetValueType(_T("boolKey")));
}

TEST(CommandJSONEntryTest, SetAndGetBytes)
{
    CommandJSONEntry entry(_T("testCommand"));
    uint8_t bytes[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };
    entry.SetBytes(_T("bytesKey"), bytes, sizeof(bytes));

    uint8_t readBytes[5];
    ASSERT_TRUE(entry.GetBytes(_T("bytesKey"), readBytes, sizeof(readBytes)));
    for (size_t i = 0; i < sizeof(bytes); ++i) {
        ASSERT_EQ(bytes[i], readBytes[i]);
    }
    ASSERT_EQ(sizeof(bytes), entry.GetBytesLength(_T("bytesKey")));
    ASSERT_EQ(CommandJSONEntry::VALUE_TYPE_BYTES, entry.GetValueType(_T("bytesKey")));

    // Test with insufficient buffer
    uint8_t smallBuf[3];
    ASSERT_FALSE(entry.GetBytes(_T("bytesKey"), smallBuf, sizeof(smallBuf)));

    // Test non-existent key
    ASSERT_FALSE(entry.GetBytes(_T("nonExistentKey"), readBytes, sizeof(readBytes)));
    ASSERT_EQ(0, entry.GetBytesLength(_T("nonExistentKey")));
}

TEST(CommandJSONEntryTest, GetValueTypeWhenKeyDoesNotExist)
{
    CommandJSONEntry entry(_T("testCommand"));
    ASSERT_EQ(CommandJSONEntry::VALUE_TYPE_NONE, entry.GetValueType(_T("nonExistentKey")));
}

TEST(CommandJSONEntryTest, HasValue)
{
    CommandJSONEntry entry(_T("testCommand"));
    ASSERT_FALSE(entry.HasValue(_T("intKey")));
    entry.Set(_T("intKey"), 123);
    ASSERT_TRUE(entry.HasValue(_T("intKey")));
}

TEST(CommandJSONEntryTest, DumpRawData)
{
    CommandJSONEntry entry(_T("testCommand"));
    entry.Set(_T("intKey"), 123);
    entry.Set(_T("stringKey"), CString(_T("testString")));

    std::vector<uint8_t> rawData;
    entry.DumpRawData(rawData);

    std::string dumpedJson(rawData.begin(), rawData.end());
    json parsedJson = json::parse(dumpedJson);

    ASSERT_TRUE(parsedJson.contains("testCommand"));
    ASSERT_TRUE(parsedJson["testCommand"].contains("intKey"));
    ASSERT_EQ(123, parsedJson["testCommand"]["intKey"]["value"].get<int>());
    ASSERT_TRUE(parsedJson["testCommand"].contains("stringKey"));
    ASSERT_EQ("testString", parsedJson["testCommand"]["stringKey"]["value"].get<std::string>());
}

