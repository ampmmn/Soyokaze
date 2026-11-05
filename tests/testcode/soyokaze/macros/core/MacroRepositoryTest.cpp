#include "stdafx.h"
#include "gtest/gtest.h"
#include "macros/core/MacroBase.h"
#include "macros/core/MacroRepository.h"

namespace {

class TestMacro : public launcherapp::macros::core::MacroBase
{
public:
	TestMacro(const std::vector<CString>& values) : mValues(values)
	{
		mName = _T("macro_repository_test");
	}

	~TestMacro() override = default;

	bool Evaluate(const std::vector<CString>& args, std::vector<CString>& results) override
	{
		UNREFERENCED_PARAMETER(args);
		results = mValues;
		return true;
	}

private:
	std::vector<CString> mValues;
};

bool RegisterTestMacro(launcherapp::macros::core::MacroRepository* repository, TestMacro*& macro, const std::vector<CString>& values)
{
	macro = new TestMacro(values);
	if (repository->RegisterMacro(macro) == false) {
		macro->Release();
		macro = nullptr;
		return false;
	}
	macro->Release();
	return true;
}

void UnregisterTestMacro(launcherapp::macros::core::MacroRepository* repository, TestMacro* macro)
{
	repository->UnregisterMacro(macro);
}

} // 名前空間

TEST(MacroRepositoryTest, QuotesEachResultAsASeparateCommandLineArgument)
{
	auto repository = launcherapp::macros::core::MacroRepository::GetInstance();
	std::vector<CString> values{
		_T("C:\\Program Files\\one.txt"),
		_T("D:\\folder two\\item&two.txt")
	};
	TestMacro* macro = nullptr;
	ASSERT_TRUE(RegisterTestMacro(repository, macro, values));

	CString text(_T("${macro_repository_test}"));
	EXPECT_TRUE(repository->Evaluate(text, true));
	EXPECT_STREQ(_T("\"C:\\Program Files\\one.txt\" \"D:\\folder two\\item&two.txt\""), (LPCTSTR)text);

	UnregisterTestMacro(repository, macro);
}

TEST(MacroRepositoryTest, JoinsUnquotedResultsWithSpacesOutsideCommandLineContext)
{
	auto repository = launcherapp::macros::core::MacroRepository::GetInstance();
	std::vector<CString> values{
		_T("C:\\Program Files\\one.txt"),
		_T("D:\\folder two\\item&two.txt")
	};
	TestMacro* macro = nullptr;
	ASSERT_TRUE(RegisterTestMacro(repository, macro, values));

	CString text(_T("${macro_repository_test}"));
	EXPECT_TRUE(repository->Evaluate(text, false));
	EXPECT_STREQ(_T("C:\\Program Files\\one.txt D:\\folder two\\item&two.txt"), (LPCTSTR)text);

	UnregisterTestMacro(repository, macro);
}

TEST(MacroRepositoryTest, EmptyResultListExpandsToEmptyText)
{
	auto repository = launcherapp::macros::core::MacroRepository::GetInstance();
	TestMacro* macro = nullptr;
	ASSERT_TRUE(RegisterTestMacro(repository, macro, {}));

	CString text(_T("${macro_repository_test}"));
	EXPECT_TRUE(repository->Evaluate(text, true));
	EXPECT_STREQ(_T(""), (LPCTSTR)text);

	UnregisterTestMacro(repository, macro);
}
