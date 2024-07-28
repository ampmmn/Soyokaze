#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/Path.h"

TEST(Path, testConstructDefault)
{
	Path path;

	bool isEmpty = path.IsEmptyPath();

	EXPECT_TRUE(isEmpty);
}

TEST(Path, testConstructString)
{
	Path path(_T("c:\\windows"));

	bool isEmpty = path.IsEmptyPath();

	EXPECT_FALSE(isEmpty);
	EXPECT_TRUE(_tcscmp(path, _T("c:\\windows")) == 0);
}

TEST(Path, testConstructModulePath)
{
	Path path(Path::MODULEFILEPATH);

	LPCTSTR p = path.FindExtension();

	EXPECT_TRUE(_tcscmp(p, _T(".exe")) == 0);
}

TEST(Path, testConstructModuleDir)
{
	Path path(Path::MODULEFILEDIR);

	bool isEmpty = path.IsEmptyPath();

	// 何かしらパスが取得できていること(ただし、取得したパスにディレクトリが存在するとは限らない)
	EXPECT_FALSE(isEmpty);
}

TEST(Path, testConstructAppDir)
{
	Path path(Path::APPDIR);

	EXPECT_TRUE(path.IsDirectory());
}

TEST(Path, testSize)
{
	Path path;

	EXPECT_EQ(MAX_PATH_NTFS, path.size());
}

TEST(Path, testShrink)
{
	Path path;

	path.Shrink();

	// 終端文字のみとなるのでサイズは1
	EXPECT_EQ(1, path.size());
}

TEST(Path, testAppend)
{
	Path path(_T("c:\\"));

	path.Append(_T("path\\to"));

	EXPECT_TRUE(_tcscmp(path, _T("c:\\path\\to")) == 0);
}

TEST(Path, testAddExtension)
{
	Path path(_T("c:\\a"));

	path.AddExtension(_T(".jpg"));

	EXPECT_TRUE(_tcscmp(path, _T("c:\\a.jpg")) == 0);
}

TEST(Path, testFindFindName)
{
	Path path(_T("c:\\a.txt"));

	LPCTSTR p = path.FindFileName();

	EXPECT_TRUE(_tcscmp(p, _T("a.txt")) == 0);
}

TEST(Path, testFindExtension)
{
	Path path(_T("c:\\a.txt"));

	LPCTSTR p = path.FindExtension();

	EXPECT_TRUE(_tcscmp(p, _T(".txt")) == 0);
}

TEST(Path, testRemoveFileSpec)
{
	Path path(_T("c:\\a.txt"));

	bool isRemoved = path.RemoveFileSpec();

	EXPECT_TRUE(isRemoved);
	EXPECT_TRUE(_tcscmp(path, _T("c:\\")) == 0);
}

TEST(Path, testRemoveExtension)
{
	Path path(_T("c:\\a.txt"));

	path.RemoveExtension();

	EXPECT_TRUE(_tcscmp(path, _T("c:\\a")) == 0);
}

TEST(Path, testFileExists)
{
	Path path(Path::MODULEFILEPATH);

	bool isExist = path.FileExists();

	EXPECT_TRUE(isExist);
}

TEST(Path, testIsDirectory)
{
	Path path(Path::MODULEFILEDIR);

	bool isExist = path.IsDirectory();

	EXPECT_TRUE(isExist);
}

TEST(Path, testIsURL)
{
	Path path(_T("https://www.yahoo.co.jp"));
	Path path2(_T("c:\\a.txt"));

	bool isURL1 = path.IsURL();
	bool isURL2 = path2.IsURL();

	EXPECT_TRUE(isURL1);
	EXPECT_FALSE(isURL2);
}

TEST(Path, testIsRelative)
{
	Path path(_T("a.txt"));
	Path path2(_T("c:\\a.txt"));

	bool isRelative1 = path.IsRelative();
	bool isRelative2 = path2.IsRelative();

	EXPECT_TRUE(isRelative1);
	EXPECT_FALSE(isRelative2);
}

TEST(Path, testCreateFromUrl)
{
	Path path;
	LPCTSTR url = _T("file:///c:/a.txt");

	HRESULT hr = path.CreateFromUrl(url);

	EXPECT_TRUE(SUCCEEDED(hr));
	EXPECT_TRUE(_tcscmp(path, _T("c:\\a.txt")) == 0);
}

