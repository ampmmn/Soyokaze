#include "stdafx.h"
#include "gtest/gtest.h"
#include "features/keywordmanager/CommandImportNameResolver.h"

using launcherapp::core::CommandImportNameResolver;

TEST(CommandImportNameResolverTest, UsesFirstAvailableNumberedName)
{
	std::vector<CString> occupiedNames = {
		_T("sample-1"),
		_T("sample-2"),
		_T("sample-4"),
	};

	auto result = CommandImportNameResolver::GetUniqueName(_T("sample"), [&](const CString& name) {
		return std::find(occupiedNames.begin(), occupiedNames.end(), name) != occupiedNames.end();
	});

	EXPECT_EQ(CString(_T("sample-3")), result);
}

TEST(CommandImportNameResolverTest, ReturnsFirstNumberedNameWhenNoneAreOccupied)
{
	auto result = CommandImportNameResolver::GetUniqueName(_T("sample"), [](const CString&) {
		return false;
	});

	EXPECT_EQ(CString(_T("sample-1")), result);
}
