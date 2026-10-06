#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/FilerParameter.h"

using launcherapp::utility::ExpandFilerParameter;

TEST(FilerParameter, ExpandTarget)
{
	CString parameter(_T("--path=$target"));

	ExpandFilerParameter(parameter, _T("C:\\work\\folder"));

	EXPECT_STREQ(_T("--path=C:\\work\\folder"), (LPCTSTR)parameter);
}

TEST(FilerParameter, ExpandTargetWithSlashSeparators)
{
	CString parameter(_T("--path=${target:s}"));

	ExpandFilerParameter(parameter, _T("C:\\work\\folder"));

	EXPECT_STREQ(_T("--path=C:/work/folder"), (LPCTSTR)parameter);
}

TEST(FilerParameter, ExpandUncTargetWithSlashSeparators)
{
	CString parameter(_T("--path=${target:s}"));

	ExpandFilerParameter(parameter, _T("\\\\server\\share\\folder"));

	EXPECT_STREQ(_T("--path=//server/share/folder"), (LPCTSTR)parameter);
}

TEST(FilerParameter, ExpandBothTargetFormats)
{
	CString parameter(_T("$target ${target:s}"));

	ExpandFilerParameter(parameter, _T("C:\\work\\folder"));

	EXPECT_STREQ(_T("C:\\work\\folder C:/work/folder"), (LPCTSTR)parameter);
}

TEST(FilerParameter, PreservePlaceholderTextInsideTargetPath)
{
	CString parameter(_T("${target:s}"));

	ExpandFilerParameter(parameter, _T("C:\\work\\$target\\${target:s}"));

	EXPECT_STREQ(_T("C:/work/$target/${target:s}"), (LPCTSTR)parameter);
}
