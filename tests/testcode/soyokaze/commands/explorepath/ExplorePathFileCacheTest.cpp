#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/explorepath/ExplorePathFileCache.h"
#include "utility/Path.h"

#include <vector>

namespace {

class TemporaryDirectory
{
public:
	~TemporaryDirectory()
	{
		for (auto& filePath : mFiles) {
			DeleteFile(filePath);
		}
		for (auto it = mDirectories.rbegin(); it != mDirectories.rend(); ++it) {
			RemoveDirectory(*it);
		}
		if (mPath.IsEmpty() == FALSE) {
			RemoveDirectory(mPath);
		}
	}

	bool Create()
	{
		TCHAR tempPath[MAX_PATH_NTFS]{};
		DWORD length = GetTempPath(_countof(tempPath), tempPath);
		if (length == 0 || length >= _countof(tempPath)) {
			return false;
		}

		TCHAR tempFile[MAX_PATH_NTFS]{};
		if (GetTempFileName(tempPath, _T("soy"), 0, tempFile) == 0) {
			return false;
		}
		if (DeleteFile(tempFile) == FALSE || CreateDirectory(tempFile, nullptr) == FALSE) {
			DeleteFile(tempFile);
			return false;
		}

		mPath = tempFile;
		return true;
	}

	CString MakeDirectory(const CString& name)
	{
		CString path = mPath + _T("\\") + name;
		if (CreateDirectory(path, nullptr) == FALSE) {
			return _T("");
		}
		mDirectories.push_back(path);
		return path;
	}

	CString MakeFile(const CString& name)
	{
		CString path = mPath + _T("\\") + name;
		if (MakeFilePath(path) == false) {
			return CString();
		}
		return path;
	}

	CString MakeFileIn(const CString& directory, const CString& name)
	{
		CString path = directory + _T("\\") + name;
		if (MakeFilePath(path) == false) {
			return CString();
		}
		return path;
	}

	CString mPath;

private:
	bool MakeFilePath(const CString& path)
	{
		HANDLE handle = CreateFile(
			path,
			GENERIC_WRITE,
			0,
			nullptr,
			CREATE_NEW,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
		if (handle == INVALID_HANDLE_VALUE) {
			return false;
		}
		CloseHandle(handle);
		mFiles.push_back(path);
		return true;
	}

	std::vector<CString> mFiles;
	std::vector<CString> mDirectories;
};

using Entry = launcherapp::commands::explorepath::ExplorePathFileCache::Entry;

static std::vector<Entry> GetEntries(
	launcherapp::commands::explorepath::ExplorePathFileCache& cache,
	const CString& path,
	bool* result = nullptr
)
{
	std::vector<Entry> entries;
	bool isOK = cache.ForEach(path, [&](const Entry& entry) {
		entries.push_back(entry);
	});
	if (result) {
		*result = isOK;
	}
	return entries;
}

static bool HasEntry(
	const std::vector<Entry>& entries,
	const CString& name,
	bool isDirectory
)
{
	for (const auto& entry : entries) {
		if (entry.mName.CompareNoCase(name) == 0 && entry.mIsDirectory == isDirectory) {
			return true;
		}
	}
	return false;
}

} // namespace

TEST(ExplorePathFileCache, EnumeratesFilesAndDirectories)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());
	ASSERT_FALSE(temp.MakeFile(_T("file.txt")).IsEmpty());
	ASSERT_FALSE(temp.MakeDirectory(_T("folder")).IsEmpty());

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	bool isOK = false;
	auto entries = GetEntries(cache, temp.mPath, &isOK);

	EXPECT_TRUE(isOK);
	EXPECT_EQ(2, entries.size());
	EXPECT_TRUE(HasEntry(entries, _T("file.txt"), false));
	EXPECT_TRUE(HasEntry(entries, _T("folder"), true));
}

TEST(ExplorePathFileCache, ReturnsEmptyForEmptyDirectory)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	bool isOK = false;
	auto entries = GetEntries(cache, temp.mPath, &isOK);

	EXPECT_TRUE(isOK);
	EXPECT_TRUE(entries.empty());
}

TEST(ExplorePathFileCache, ReturnsFalseForMissingDirectory)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	bool isOK = true;
	auto entries = GetEntries(cache, temp.mPath + _T("\\missing"), &isOK);

	EXPECT_FALSE(isOK);
	EXPECT_TRUE(entries.empty());
}

TEST(ExplorePathFileCache, ReusesCacheForSameDirectory)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());
	ASSERT_FALSE(temp.MakeFile(_T("first.txt")).IsEmpty());

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	ASSERT_EQ(1, GetEntries(cache, temp.mPath).size());
	ASSERT_FALSE(temp.MakeFile(_T("second.txt")).IsEmpty());

	auto entries = GetEntries(cache, temp.mPath);
	EXPECT_EQ(1, entries.size());
	EXPECT_TRUE(HasEntry(entries, _T("first.txt"), false));
	EXPECT_FALSE(HasEntry(entries, _T("second.txt"), false));
}

TEST(ExplorePathFileCache, ClearReloadsDirectory)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());
	ASSERT_FALSE(temp.MakeFile(_T("first.txt")).IsEmpty());

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	GetEntries(cache, temp.mPath);
	ASSERT_FALSE(temp.MakeFile(_T("second.txt")).IsEmpty());

	cache.Clear();
	auto entries = GetEntries(cache, temp.mPath);
	EXPECT_EQ(2, entries.size());
	EXPECT_TRUE(HasEntry(entries, _T("second.txt"), false));
}

TEST(ExplorePathFileCache, ReusesCacheForNormalizedDirectoryPath)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());
	ASSERT_FALSE(temp.MakeFile(_T("first.txt")).IsEmpty());

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	GetEntries(cache, temp.mPath);
	ASSERT_FALSE(temp.MakeFile(_T("second.txt")).IsEmpty());

	CString normalizedPath = temp.mPath;
	normalizedPath.Replace(_T('\\'), _T('/'));
	normalizedPath += _T("\\");
	auto entries = GetEntries(cache, normalizedPath);

	EXPECT_EQ(1, entries.size());
	EXPECT_FALSE(HasEntry(entries, _T("second.txt"), false));
}

TEST(ExplorePathFileCache, SwitchesCacheWhenDirectoryChanges)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());
	CString firstDirectory = temp.MakeDirectory(_T("first"));
	CString secondDirectory = temp.MakeDirectory(_T("second"));
	ASSERT_FALSE(firstDirectory.IsEmpty());
	ASSERT_FALSE(secondDirectory.IsEmpty());
	ASSERT_FALSE(temp.MakeFileIn(firstDirectory, _T("first.txt")).IsEmpty());
	ASSERT_FALSE(temp.MakeFileIn(secondDirectory, _T("second.txt")).IsEmpty());

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	auto firstEntries = GetEntries(cache, firstDirectory);
	auto secondEntries = GetEntries(cache, secondDirectory);

	EXPECT_TRUE(HasEntry(firstEntries, _T("first.txt"), false));
	EXPECT_FALSE(HasEntry(firstEntries, _T("second.txt"), false));
	EXPECT_TRUE(HasEntry(secondEntries, _T("second.txt"), false));
	EXPECT_FALSE(HasEntry(secondEntries, _T("first.txt"), false));
}

TEST(ExplorePathFileCache, DoesNotReusePartialCacheForLargeDirectory)
{
	TemporaryDirectory temp;
	ASSERT_TRUE(temp.Create());

	constexpr int entryCount = 4097;
	for (int i = 0; i < entryCount; ++i) {
		CString name;
		name.Format(_T("entry%04d.txt"), i);
		ASSERT_FALSE(temp.MakeFile(name).IsEmpty());
	}

	launcherapp::commands::explorepath::ExplorePathFileCache cache;
	auto firstEntries = GetEntries(cache, temp.mPath);
	ASSERT_EQ(entryCount, firstEntries.size());
	ASSERT_FALSE(temp.MakeFile(_T("added-after-enumeration.txt")).IsEmpty());

	auto secondEntries = GetEntries(cache, temp.mPath);
	EXPECT_EQ(entryCount + 1, secondEntries.size());
	EXPECT_TRUE(HasEntry(secondEntries, _T("added-after-enumeration.txt"), false));
}
