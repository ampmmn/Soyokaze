#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/office_favorites/OfficeFavorites.h"

using OfficeFavorites = launcherapp::commands::office_favorites::OfficeFavorites;
using OfficeFavoriteItem = launcherapp::commands::office_favorites::OfficeFavoriteItem;
using OfficeAppType = launcherapp::commands::office_favorites::OfficeAppType;

// ピン止めされたレジストリ値からパスを取り出せること
TEST(OfficeFavoritesTest, ParseRegistryValuePinned)
{
	CString path;
	EXPECT_TRUE(OfficeFavorites::ParseRegistryValue(_T("[F00000001][T01DD585EE7AA9830][O00000000]*C:\\Users\\test\\Book1.xlsx"), path));
	EXPECT_EQ(_T("C:\\Users\\test\\Book1.xlsx"), path);
}

// ピン止めされていないレジストリ値は対象外であること
TEST(OfficeFavoritesTest, ParseRegistryValueNotPinned)
{
	CString path;
	EXPECT_FALSE(OfficeFavorites::ParseRegistryValue(_T("[F00000000][T01DD58489E104960][O00000000]*C:\\Users\\test\\Book1.xlsx"), path));
}

// 形式が不正なレジストリ値は対象外であること
TEST(OfficeFavoritesTest, ParseRegistryValueMalformed)
{
	CString path;
	// '*' が無い
	EXPECT_FALSE(OfficeFavorites::ParseRegistryValue(_T("[F00000001][T01DD585EE7AA9830][O00000000]"), path));
	// パスが空
	EXPECT_FALSE(OfficeFavorites::ParseRegistryValue(_T("[F00000001]*"), path));
}

// JSONからピン止めされた対象アプリの項目のみを抽出すること
TEST(OfficeFavoritesTest, ParseAggMruJsonPicksPinnedTargetApps)
{
	const char* json = R"({"documents":{"items":[
		{"app":"Excel","title":"Book1","extension":"xlsx","is_pinned":true,"url":"https://d.docs.live.net/xxx/Book1.xlsx"},
		{"app":"Excel","title":"Book2","extension":"xlsx","is_pinned":false,"url":"https://d.docs.live.net/xxx/Book2.xlsx"},
		{"app":"OneNote","title":"Note","extension":"one","is_pinned":true,"url":"https://d.docs.live.net/xxx/Note"},
		{"app":"Word","title":"","extension":"","is_pinned":true,"url":"https://d.docs.live.net/xxx/Doc%20A.docx"}
	]}})";

	std::vector<OfficeFavoriteItem> items;
	EXPECT_TRUE(OfficeFavorites::ParseAggMruJson(json, items));
	ASSERT_EQ(2, (int)items.size());

	EXPECT_EQ(OfficeAppType::Excel, items[0].GetAppType());
	EXPECT_EQ(_T("Book1.xlsx"), items[0].GetName());
	EXPECT_EQ(_T(".xlsx"), items[0].GetExtension());
	EXPECT_TRUE(items[0].IsURL());

	// 表示名はURLの末尾要素をデコードしたものになる
	EXPECT_EQ(OfficeAppType::Word, items[1].GetAppType());
	EXPECT_EQ(_T("Doc A.docx"), items[1].GetName());
}

// http:// と https:// のみURLと判定し、大文字小文字は区別しないこと
TEST(OfficeFavoriteItemTest, IsURL)
{
	OfficeFavoriteItem item;

	item.mPath = _T("https://d.docs.live.net/xxx/Book1.xlsx");
	EXPECT_TRUE(item.IsURL());

	item.mPath = _T("HTTP://example.com/a.docx");
	EXPECT_TRUE(item.IsURL());

	item.mPath = _T("C:\\Users\\test\\Book1.xlsx");
	EXPECT_FALSE(item.IsURL());

	item.mPath = _T("file:///C:/Users/test/Book1.xlsx");
	EXPECT_FALSE(item.IsURL());
}

// ローカルパスの表示名と拡張子はパスから求められること
TEST(OfficeFavoriteItemTest, LocalNameAndExtension)
{
	OfficeFavoriteItem item;
	item.mPath = _T("C:\\Users\\test\\Sub Dir\\Slide.pptx");
	EXPECT_EQ(_T("Slide.pptx"), item.GetName());
	EXPECT_EQ(_T(".pptx"), item.GetExtension());

	// 拡張子が無いファイルは空になる
	item.mPath = _T("C:\\Users\\test\\memo");
	EXPECT_EQ(_T("memo"), item.GetName());
	EXPECT_TRUE(item.GetExtension().IsEmpty());
}

// URLの表示名はクエリ文字列を除き、パーセントデコードされること
TEST(OfficeFavoriteItemTest, UrlNameIgnoresQueryAndDecodes)
{
	OfficeFavoriteItem item;
	item.mPath = _T("https://example.sharepoint.com/sites/a/Doc%20A.docx?web=1&x=y");
	EXPECT_EQ(_T("Doc A.docx"), item.GetName());
	EXPECT_EQ(_T(".docx"), item.GetExtension());
}

// JSON の UTF-8 の title が正しく読めて、表示名に反映されること
TEST(OfficeFavoritesTest, ParseAggMruJsonReadsUtf8Title)
{
	// title: 反例1 / url のファイル名は UTF-8 のパーセントエンコード
	const char* json = R"({"documents":{"items":[
		{"app":"Excel","title":"\u53cd\u4f8b1","extension":"xlsx","is_pinned":true,"url":"https://d.docs.live.net/xxx/%E5%8F%8D%E4%BE%8B1.xlsx"}
	]}})";

	std::vector<OfficeFavoriteItem> items;
	ASSERT_TRUE(OfficeFavorites::ParseAggMruJson(json, items));
	ASSERT_EQ(1, (int)items.size());
	EXPECT_STREQ(L"\u53CD\u4F8B1.xlsx", (LPCTSTR)items[0].GetName());
	EXPECT_EQ(_T(".xlsx"), items[0].GetExtension());
}

// URL の表示名は title を優先し、拡張子は JSON の extension を使うこと(.aspx の URL)
TEST(OfficeFavoritesTest, ParseAggMruJsonUsesTitleForAspxUrl)
{
	const char* json = R"({"documents":{"items":[
		{"app":"Word","title":"Report","extension":"docx","is_pinned":true,"url":"https://example.sharepoint.com/sites/a/_layouts/15/Doc.aspx?sourcedoc=%7B1%7D"}
	]}})";

	std::vector<OfficeFavoriteItem> items;
	ASSERT_TRUE(OfficeFavorites::ParseAggMruJson(json, items));
	ASSERT_EQ(1, (int)items.size());
	EXPECT_EQ(_T("Report.docx"), items[0].GetName());
	EXPECT_EQ(_T(".docx"), items[0].GetExtension());
}

// title が空の場合は mName を持たず、URL の末尾要素から表示名を求めること
TEST(OfficeFavoritesTest, ParseAggMruJsonFallsBackToUrlWhenTitleEmpty)
{
	const char* json = R"({"documents":{"items":[
		{"app":"PowerPoint","title":"","extension":"pptx","is_pinned":true,"url":"https://d.docs.live.net/xxx/Slide%20A.pptx"}
	]}})";

	std::vector<OfficeFavoriteItem> items;
	ASSERT_TRUE(OfficeFavorites::ParseAggMruJson(json, items));
	ASSERT_EQ(1, (int)items.size());
	EXPECT_TRUE(items[0].mName.IsEmpty());
	EXPECT_EQ(_T("Slide A.pptx"), items[0].GetName());
	EXPECT_EQ(_T(".pptx"), items[0].GetExtension());
}

// URL のデコードは UTF-8 として行われること
TEST(OfficeFavoritesTest, DecodeUrlUsesUtf8)
{
	// URL全体がデコードされること
	CString decoded = OfficeFavorites::DecodeUrl(CString(_T("https://x/%E5%8F%8D%E4%BE%8B1.xlsx")));
	EXPECT_STREQ(L"https://x/\u53CD\u4F8B1.xlsx", (LPCTSTR)decoded);
}

// アプリ名とアプリ種別の相互変換が行えること
TEST(OfficeFavoritesTest, AppTypeNameConversion)
{
	OfficeAppType type{};
	EXPECT_TRUE(OfficeFavorites::AppTypeFromName(_T("Word"), type));
	EXPECT_EQ(OfficeAppType::Word, type);
	EXPECT_EQ(_T("PowerPoint"), OfficeFavorites::AppTypeName(OfficeAppType::PowerPoint));

	// 対象外のアプリ名は失敗すること
	EXPECT_FALSE(OfficeFavorites::AppTypeFromName(_T("OneNote"), type));
}

// 不正なJSONは解析失敗となり、項目を追加しないこと
TEST(OfficeFavoritesTest, ParseAggMruJsonInvalid)
{
	std::vector<OfficeFavoriteItem> items;
	EXPECT_FALSE(OfficeFavorites::ParseAggMruJson("{invalid", items));
	EXPECT_TRUE(items.empty());
}

// 監視対象のUser MRUキーがアプリごとに返されること
TEST(OfficeFavoritesTest, GetUserMRUKeyPathsReturnsThreeApps)
{
	std::vector<CString> keyPaths;
	OfficeFavorites::GetUserMRUKeyPaths(keyPaths);
	ASSERT_EQ(3, (int)keyPaths.size());
	EXPECT_EQ(_T("Software\\Microsoft\\Office\\16.0\\Excel\\User MRU"), keyPaths[0]);
}
