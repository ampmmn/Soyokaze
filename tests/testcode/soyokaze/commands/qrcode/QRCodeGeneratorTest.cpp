#include "stdafx.h"
#include "commands/qrcode/QRCodeGenerator.h"
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

using launcherapp::commands::qrcode::QRCodeGenerator;

TEST(QRCodeGeneratorTest, GeneratesQrCodeForUtf8Text)
{
	QRCodeGenerator generator;
	EXPECT_TRUE(generator.Generate(_T("こんにちは QR Code"), QRCodeGenerator::LEVEL_M));
	EXPECT_TRUE(generator.IsValid());
}

TEST(QRCodeGeneratorTest, GeneratesQrCodeForJapaneseTextEndingInKanjiLikeUtf8Byte)
{
	QRCodeGenerator generator;
	EXPECT_TRUE(generator.Generate(_T("突撃隣の晩ごはん"), QRCodeGenerator::LEVEL_M));
	EXPECT_TRUE(generator.IsValid());
}

TEST(QRCodeGeneratorTest, GeneratesQrCodeForJapaneseTextUsingMultipleDataBlocks)
{
	QRCodeGenerator generator;
	EXPECT_TRUE(generator.Generate(_T("こんにちはこれはテストです。"), QRCodeGenerator::LEVEL_M));
	EXPECT_TRUE(generator.IsValid());
}

TEST(QRCodeGeneratorTest, RejectsEmptyTextAndInvalidErrorCorrectionLevel)
{
	QRCodeGenerator generator;
	EXPECT_FALSE(generator.Generate(_T(""), QRCodeGenerator::LEVEL_M));
	EXPECT_FALSE(generator.Generate(_T("text"), 4));
	EXPECT_FALSE(generator.IsValid());
}

TEST(QRCodeGeneratorTest, RejectsInvalidUtf16)
{
	CString text;
	text.AppendChar(static_cast<TCHAR>(0xD800));

	QRCodeGenerator generator;
	EXPECT_FALSE(generator.Generate(text, QRCodeGenerator::LEVEL_M));
	EXPECT_FALSE(generator.IsValid());
}

TEST(QRCodeGeneratorTest, SavesSvg)
{
	QRCodeGenerator generator;
	ASSERT_TRUE(generator.Generate(_T("こんにちは QR Code"), QRCodeGenerator::LEVEL_M));

	TCHAR tempPath[MAX_PATH]{};
	ASSERT_NE(0u, ::GetTempPath(MAX_PATH, tempPath));
	TCHAR tempFilePath[MAX_PATH]{};
	ASSERT_NE(0u, ::GetTempFileName(tempPath, _T("qr"), 0, tempFilePath));

	EXPECT_TRUE(generator.SaveSvg(tempFilePath));
	std::ifstream input(std::filesystem::path(tempFilePath), std::ios::binary);
	EXPECT_TRUE(input.is_open());
	if (input.is_open()) {
		std::string svg{std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>()};
		EXPECT_NE(svg.find("<svg "), std::string::npos);
		EXPECT_NE(svg.find("<path "), std::string::npos);
		EXPECT_NE(svg.find("</svg>"), std::string::npos);
	}
	::DeleteFile(tempFilePath);
}

TEST(QRCodeGeneratorTest, RejectsTextThatExceedsTheMaximumCapacity)
{
	CString text;
	for (int i = 0; i < 6000; ++i) {
		text += _T('a');
	}

	QRCodeGenerator generator;
	EXPECT_FALSE(generator.Generate(text, QRCodeGenerator::LEVEL_L));
	EXPECT_FALSE(generator.IsValid());
}
