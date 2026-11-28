#include "stdafx.h"
#include "gtest/gtest.h"
#include "WorkspaceIndex.h"
#include <utility>

using namespace workspaceplugin;

TEST(WorkspaceStringPoolTest, InternReturnsSameIdForDuplicateStrings)
{
 StringPool strings;

 const StringId firstId = strings.Intern(L"src");
 const StringId secondId = strings.Intern(L"src");

 EXPECT_EQ(firstId, secondId);
 EXPECT_EQ(strings.Size(), 1u);
 EXPECT_EQ(strings.Get(firstId), L"src");
 EXPECT_EQ(strings.GetUtf8(firstId), "src");
}

TEST(WorkspaceStringPoolTest, InternPreservesUnicodeAndInvalidIdsReturnEmptyViews)
{
 StringPool strings;
 const StringId id = strings.Intern(L"日本語.txt");

 EXPECT_EQ(strings.GetUtf8(id), std::string_view(reinterpret_cast<const char*>(u8"日本語.txt")));
 EXPECT_TRUE(strings.Get(InvalidStringId).empty());
 EXPECT_TRUE(strings.GetUtf8(InvalidStringId).empty());
}

TEST(WorkspaceStringPoolTest, MovePreservesIdsAndStringViews)
{
 StringPool strings;
 const StringId id = strings.Intern(L"main.cpp");

 StringPool movedStrings = std::move(strings);

 EXPECT_EQ(movedStrings.Get(id), L"main.cpp");
 EXPECT_EQ(movedStrings.GetUtf8(id), "main.cpp");
 EXPECT_EQ(movedStrings.Intern(L"main.cpp"), id);
}

TEST(WorkspaceIndexTest, BuildFullPathAndRelativePathFollowParentIds)
{
 StringPool strings;
 const StringId projectId = strings.Intern(L"ProjectA");
 const StringId sourceId = strings.Intern(L"src");
 const StringId fileId = strings.Intern(L"main.cpp");
 const std::vector<Entry> entries = {
  { InvalidEntryId, projectId, true, 0 },
  { 0, sourceId, true, 0 },
  { 1, fileId, false, 0 },
 };
 const std::vector<RootInfo> roots = {
  { std::filesystem::path(L"C:\\Work"), 0 },
 };

 EXPECT_EQ(BuildRelativePath(2, entries, strings), std::filesystem::path(L"ProjectA\\src\\main.cpp"));
 EXPECT_EQ(strings.GetUtf8(entries[2].nameId), "main.cpp");
 EXPECT_EQ(BuildFullPath(2, entries, roots, strings),
  std::filesystem::path(L"C:\\Work\\ProjectA\\src\\main.cpp"));
}

TEST(WorkspaceIndexTest, BuildFullPathRejectsInvalidOrCyclicParentIds)
{
 StringPool strings;
 const StringId nameId = strings.Intern(L"folder");
 const std::vector<RootInfo> roots = {
  { std::filesystem::path(L"C:\\Work"), 0 },
 };
 const std::vector<Entry> cyclicEntries = {
  { 0, nameId, true, 0 },
 };

 EXPECT_TRUE(BuildRelativePath(0, cyclicEntries, strings).empty());
 EXPECT_TRUE(BuildFullPath(1, cyclicEntries, roots, strings).empty());
}
