#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/CharConverter.h"

using namespace launcherapp::utility;


class CharConverterTest : public ::testing::Test {
protected:
    CharConverter* converter;

    void SetUp() override {
        converter = new CharConverter(CP_UTF8);
    }

    void TearDown() override {
        delete converter;
    }
};

TEST_F(CharConverterTest, ConvertCharToCString_Success) {
    const char* src = "Hello, World!";
    CString dst;

    EXPECT_NO_THROW({
        converter->Convert(src, dst);
    });

    EXPECT_STREQ(dst, L"Hello, World!");
}

TEST_F(CharConverterTest, ConvertCharToCString_FailIfInvalidChars) {
    const char* src = "Hello, \xFF\xFF!";
    CString dst;

    EXPECT_THROW({
        converter->Convert(src, dst, true);
    }, CharConverter::Exception);
}

TEST_F(CharConverterTest, ConvertCStringToCStringA) {
    CString src = L"Hello, World!";
    CStringA dst;

    EXPECT_NO_THROW({
        converter->Convert(src, dst);
    });

    EXPECT_STREQ(dst, "Hello, World!");
}

TEST_F(CharConverterTest, ConvertCStringToStdString) {
    CString src = L"Hello, World!";
    std::string dst;

    EXPECT_NO_THROW({
        converter->Convert(src, dst);
    });

    EXPECT_EQ(dst, "Hello, World!");
}

TEST_F(CharConverterTest, ScalarToUTF8_OneByte) {
    char dst[4] = {0};
    int len = CharConverter::ScalarToUTF8(0x24, dst); // U+0024 ($)

    EXPECT_EQ(len, 1);
    EXPECT_EQ(dst[0], 0x24);
}

TEST_F(CharConverterTest, ScalarToUTF8_TwoBytes) {
    char dst[4] = {0};
    int len = CharConverter::ScalarToUTF8(0xA2, dst); // U+00A2 (¢)

    EXPECT_EQ(len, 2);
    EXPECT_EQ(dst[0], (char)0xC2);
    EXPECT_EQ(dst[1], (char)0xA2);
}

TEST_F(CharConverterTest, ScalarToUTF8_ThreeBytes) {
    char dst[4] = {0};
    int len = CharConverter::ScalarToUTF8(0x20AC, dst); // U+20AC (€)

    EXPECT_EQ(len, 3);
    EXPECT_EQ(dst[0], (char)0xE2);
    EXPECT_EQ(dst[1], (char)0x82);
    EXPECT_EQ(dst[2], (char)0xAC);
}

TEST_F(CharConverterTest, ScalarToUTF8_FourBytes) {
    char dst[4] = {0};
    int len = CharConverter::ScalarToUTF8(0x1F600, dst); // U+1F600 (😀)

    EXPECT_EQ(len, 4);
    EXPECT_EQ(dst[0], (char)0xF0);
    EXPECT_EQ(dst[1], (char)0x9F);
    EXPECT_EQ(dst[2], (char)0x98);
    EXPECT_EQ(dst[3], (char)0x80);
}


TEST_F(CharConverterTest, UTF16ToUTF8_StaticMethod_ValidInput) {
    CStringW utf16Input = L"テスト";
    std::string utf8Output;

    EXPECT_NO_THROW({
        utf8Output = CharConverter::UTF2UTF(utf16Input);
    });

    EXPECT_EQ(utf8Output, (const char*)u8"テスト");
}

TEST_F(CharConverterTest, UTF16ToUTF8_StaticMethod_WithDst_ValidInput) {
    CStringW utf16Input = L"テスト";
    std::string utf8Output;

    EXPECT_NO_THROW({
        CharConverter::UTF2UTF(utf16Input, utf8Output);
    });

    EXPECT_EQ(utf8Output, (const char*)u8"テスト");
}

TEST_F(CharConverterTest, UTF16ToUTF8_StaticMethod_WithWString_ValidInput) {
    std::wstring utf16Input = L"テスト";
    std::string utf8Output;

    EXPECT_NO_THROW({
        CharConverter::UTF2UTF(utf16Input, utf8Output);
    });

    EXPECT_EQ(utf8Output, (const char*)u8"テスト");
}

TEST_F(CharConverterTest, UTF8ToUTF16_StaticMethod_ValidInput) {
    std::string utf8Input = (const char*)u8"テスト";
    CStringW utf16Output;

    EXPECT_NO_THROW({
        utf16Output = CharConverter::UTF2UTF(utf8Input);
    });

    EXPECT_EQ(utf16Output, L"テスト");
}

TEST_F(CharConverterTest, UTF8ToUTF16_StaticMethod_WithDst_ValidInput) {
    std::string utf8Input = (const char*)u8"テスト";
    CStringW utf16Output;

    EXPECT_NO_THROW({
        CharConverter::UTF2UTF(utf8Input, utf16Output);
    });

    EXPECT_EQ(utf16Output, L"テスト");
}

TEST_F(CharConverterTest, UTF8ToUTF16_StaticMethod_WithWString_ValidInput) {
    std::string utf8Input = (const char*)u8"テスト";
    std::wstring utf16Output;

    EXPECT_NO_THROW({
        CharConverter::UTF2UTF(utf8Input, utf16Output);
    });

    EXPECT_EQ(utf16Output, L"テスト");
}

TEST_F(CharConverterTest, UTF16ToUTF8_InvalidInput) {
    CStringW utf16Input = L"\xD800"; // 不正なサロゲートペア
    std::string utf8Output;

    // 文字列が不正でもとくに実装上は例外を発生させていない
    EXPECT_NO_THROW({
        CharConverter::UTF2UTF(utf16Input);
    });
}

TEST_F(CharConverterTest, UTF8ToUTF16_InvalidInput) {
    std::string utf8Input = "\xFF"; // 不正なUTF-8シーケンス
    CStringW utf16Output;

    EXPECT_NO_THROW({
        utf16Output = CharConverter::UTF2UTF(utf8Input);
    });

    EXPECT_EQ(utf16Output, L""); // 不正な入力は空文字列を返す
}
