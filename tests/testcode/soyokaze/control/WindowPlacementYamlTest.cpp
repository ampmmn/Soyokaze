#include "stdafx.h"
#include "gtest/gtest.h"
#include "control/WindowPlacementYaml.h"
#include "utility/Base64.h"

namespace {

// 任意のバイト列をBase64エンコードした文字列を取得する
std::string EncodeBytes(const std::vector<uint8_t>& bytes)
{
	CString encoded = utility::base64::EncodeBase64(bytes);
	std::string result;
	for (int i = 0; i < encoded.GetLength(); ++i) {
		result.push_back(static_cast<char>(encoded[i]));
	}
	return result;
}

// テスト用のWINDOWPLACEMENTを生成する
WINDOWPLACEMENT CreatePlacement(LONG left, LONG top, LONG right, LONG bottom)
{
	WINDOWPLACEMENT placement{};
	placement.length = sizeof(WINDOWPLACEMENT);
	placement.flags = 0;
	placement.showCmd = SW_SHOWNORMAL;
	placement.rcNormalPosition = RECT{ left, top, right, bottom };
	return placement;
}

// WINDOWPLACEMENTをBase64エンコードした文字列を取得する
std::string EncodePlacement(const WINDOWPLACEMENT& placement)
{
	std::vector<uint8_t> bytes(sizeof(WINDOWPLACEMENT));
	memcpy(bytes.data(), &placement, sizeof(WINDOWPLACEMENT));
	return EncodeBytes(bytes);
}

// ASCII文字列をstd::stringへ変換する
std::string ToAscii(const wchar_t* text)
{
	std::string result;
	for (const wchar_t* p = text; *p != L'\0'; ++p) {
		result.push_back(static_cast<char>(*p));
	}
	return result;
}

constexpr wchar_t MONITOR_ID[] = L"d1c4cbd23a41e89c5f4d2e77134a5d4d7f9e5fba";

}

TEST(WindowPlacementYamlTest, RoundTrip_KeepsPositionsSeparatedByWindowName)
{
	// 同じモニター構成識別子でもウインドウ種別ごとに独立した位置情報を持つこと
	WindowPlacementYaml::WindowPlacementMap placements;
	placements[L"Window"][MONITOR_ID] = CreatePlacement(0, 0, 600, 300);
	placements[L"ManualWindow"][MONITOR_ID] = CreatePlacement(100, 100, 800, 700);
	placements[L"ClipboardPreview"][MONITOR_ID] = CreatePlacement(600, 0, 1000, 400);

	auto yaml = WindowPlacementYaml::Emit(placements);
	ASSERT_FALSE(yaml.empty());

	WindowPlacementYaml::WindowPlacementMap restored;
	ASSERT_TRUE(WindowPlacementYaml::Parse(yaml, restored));
	ASSERT_EQ(3, restored.size());

	ASSERT_EQ(1, restored[L"Window"].size());
	EXPECT_EQ(600, restored[L"Window"][MONITOR_ID].rcNormalPosition.right);
	EXPECT_EQ(800, restored[L"ManualWindow"][MONITOR_ID].rcNormalPosition.right);
	EXPECT_EQ(1000, restored[L"ClipboardPreview"][MONITOR_ID].rcNormalPosition.right);

	// 出力した内容にウインドウ名が含まれること
	EXPECT_NE(std::string::npos, yaml.find("ManualWindow"));
	EXPECT_NE(std::string::npos, yaml.find("ClipboardPreview"));
}

TEST(WindowPlacementYamlTest, RoundTrip_KeepsMultipleMonitorConfigurations)
{
	WindowPlacementYaml::WindowPlacementMap placements;
	placements[L"Window"][L"aaa"] = CreatePlacement(0, 0, 600, 300);
	placements[L"Window"][L"bbb"] = CreatePlacement(1920, 0, 2520, 300);
	placements[L"ManualWindow"][L"aaa"] = CreatePlacement(10, 10, 810, 610);

	auto yaml = WindowPlacementYaml::Emit(placements);
	ASSERT_FALSE(yaml.empty());

	WindowPlacementYaml::WindowPlacementMap restored;
	ASSERT_TRUE(WindowPlacementYaml::Parse(yaml, restored));
	ASSERT_EQ(1, restored[L"ManualWindow"].size());
	EXPECT_EQ(2, restored[L"Window"].size());
	EXPECT_EQ(1920, restored[L"Window"][L"bbb"].rcNormalPosition.left);
	EXPECT_EQ(10, restored[L"ManualWindow"][L"aaa"].rcNormalPosition.left);
}

TEST(WindowPlacementYamlTest, Parse_LegacyFlatFormat_IsTreatedAsMainWindow)
{
	// ウインドウ種別を持たない旧形式のYAMLはメインウインドク名下として扱う
	std::string yaml;
	yaml += "LegacyId: \"";
	yaml += EncodePlacement(CreatePlacement(11, 22, 611, 322));
	yaml += "\"\n";

	WindowPlacementYaml::WindowPlacementMap restored;
	ASSERT_TRUE(WindowPlacementYaml::Parse(yaml, restored));
	ASSERT_EQ(1, restored.size());

	std::wstring mainWindowName = WindowPlacementYaml::GetMainWindowName();
	ASSERT_EQ(1, restored[mainWindowName].size());
	EXPECT_EQ(11, restored[mainWindowName][L"LegacyId"].rcNormalPosition.left);
	EXPECT_EQ(322, restored[mainWindowName][L"LegacyId"].rcNormalPosition.bottom);
}

TEST(WindowPlacementYamlTest, Parse_LegacyFlatFormat_CoexistsWithWindowSections)
{
	// 旧形式のエントリとウインドウ種別ごとのエントリが混在していても取りこぼさないこと
	std::string yaml;
	yaml += "LegacyId: \"";
	yaml += EncodePlacement(CreatePlacement(1, 2, 3, 4));
	yaml += "\"\n";
	yaml += "ManualWindow:\n  ";
	yaml += ToAscii(MONITOR_ID);
	yaml += ": \"";
	yaml += EncodePlacement(CreatePlacement(5, 6, 7, 8));
	yaml += "\"\n";

	WindowPlacementYaml::WindowPlacementMap restored;
	ASSERT_TRUE(WindowPlacementYaml::Parse(yaml, restored));
	ASSERT_EQ(2, restored.size());

	std::wstring mainWindowName = WindowPlacementYaml::GetMainWindowName();
	ASSERT_EQ(1, restored[mainWindowName].size());
	EXPECT_EQ(1, restored[mainWindowName][L"LegacyId"].rcNormalPosition.left);
	ASSERT_EQ(1, restored[L"ManualWindow"].size());
	EXPECT_EQ(5, restored[L"ManualWindow"][MONITOR_ID].rcNormalPosition.left);
}

TEST(WindowPlacementYamlTest, Parse_IgnoresDefaultKeyAndInvalidEntries)
{
	// defaultキー、Base64として不正な値、サイズ不一致のエントリは無視されること
	std::string invalidBase64 = EncodeBytes(std::vector<uint8_t>{ 1, 2, 3, 4 });

	std::string yaml;
	yaml += "ManualWindow:\n";
	yaml += "  default: \"";
	yaml += EncodePlacement(CreatePlacement(9, 9, 109, 209));
	yaml += "\"\n";
	yaml += "  ";
	yaml += ToAscii(MONITOR_ID);
	yaml += ": \"!!!!\"\n";
	yaml += "  aaa: \"";
	yaml += invalidBase64;
	yaml += "\"\n";
	yaml += "  bbb: \"";
	yaml += EncodePlacement(CreatePlacement(30, 40, 630, 340));
	yaml += "\"\n";

	WindowPlacementYaml::WindowPlacementMap restored;
	ASSERT_TRUE(WindowPlacementYaml::Parse(yaml, restored));
	ASSERT_EQ(1, restored.size());
	ASSERT_EQ(1, restored[L"ManualWindow"].size());
	EXPECT_EQ(30, restored[L"ManualWindow"][L"bbb"].rcNormalPosition.left);
}

TEST(WindowPlacementYamlTest, Parse_InvalidYaml_ReturnsFalse)
{
	WindowPlacementYaml::WindowPlacementMap restored;
	EXPECT_FALSE(WindowPlacementYaml::Parse("", restored));
	EXPECT_FALSE(WindowPlacementYaml::Parse("- a\n- b\n", restored));
	EXPECT_FALSE(WindowPlacementYaml::Parse("key: [1, 2\n", restored));
}

TEST(WindowPlacementYamlTest, Emit_EmptyOrNonAsciiName_ReturnsEmpty)
{
	WindowPlacementYaml::WindowPlacementMap placements;
	EXPECT_TRUE(WindowPlacementYaml::Emit(placements).empty());

	placements[L"ウインドウ"][MONITOR_ID] = CreatePlacement(0, 0, 100, 100);
	EXPECT_TRUE(WindowPlacementYaml::Emit(placements).empty());
}

TEST(WindowPlacementYamlTest, IsValidWindowName_AcceptsAsciiOnly)
{
	EXPECT_TRUE(WindowPlacementYaml::IsValidWindowName(L"Window"));
	EXPECT_TRUE(WindowPlacementYaml::IsValidWindowName(L"ManualWindow"));
	EXPECT_TRUE(WindowPlacementYaml::IsValidWindowName(L"ClipboardPreview"));
	EXPECT_FALSE(WindowPlacementYaml::IsValidWindowName(L""));
	EXPECT_FALSE(WindowPlacementYaml::IsValidWindowName(L"ウインドウ"));
}