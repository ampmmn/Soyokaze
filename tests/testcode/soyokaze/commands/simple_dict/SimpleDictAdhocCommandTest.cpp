#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/simple_dict/SimpleDictAdhocCommand.h"
#include "commands/simple_dict/SimpleDictParam.h"
#include "macros/core/MacroIF.h"
#include "macros/core/MacroRepository.h"

using namespace launcherapp::commands::simple_dict;
using Action = launcherapp::actions::core::Action;
using launcherapp::macros::core::MacroIF;
using launcherapp::macros::core::MacroRepository;

namespace {

class TestMacro : public MacroIF
{
public:
	CString GetName() override
	{
		return _T("Issue331TestMacro");
	}

	CString GetDescription() override
	{
		return _T("");
	}

	bool Evaluate(const std::vector<CString>& args, CString& result) override
	{
		UNREFERENCED_PARAMETER(args);
		++mEvaluateCount;
		result = _T("expanded");
		return true;
	}

	uint32_t AddRef() override
	{
		return (uint32_t)InterlockedIncrement(&mRefCount);
	}

	uint32_t Release() override
	{
		auto count = (uint32_t)InterlockedDecrement(&mRefCount);
		if (count == 0) {
			delete this;
		}
		return count;
	}

	LONG GetEvaluateCount() const
	{
		return mEvaluateCount;
	}

private:
	volatile LONG mRefCount{1};
	volatile LONG mEvaluateCount{0};
};

class SimpleDictAdhocCommandTest : public testing::Test
{
protected:
	void SetUp() override
	{
		mMacro = new TestMacro();
		MacroRepository::GetInstance()->RegisterMacro(mMacro);
	}

	void TearDown() override
	{
		MacroRepository::GetInstance()->UnregisterMacro(mMacro);
		mMacro->Release();
	}

	TestMacro* mMacro{nullptr};
};

}

TEST_F(SimpleDictAdhocCommandTest, CopyActionExpandsMacrosWhenEnabled)
{
	SimpleDictParam param;
	param.mNameFormat = _T("Name");
	param.mDescriptionFormat = _T("Description");
	param.mActionType = 2;
	param.mIsExpandMacro = TRUE;
	param.mAfterCommandParam = _T("$value");

	Record record(_T("key"), _T("$Issue331TestMacro"));
	SimpleDictAdhocCommand command(param, record);

	Action* action = nullptr;
	ASSERT_TRUE(command.GetAction(HOTKEY_ATTR(0, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	action->Release();

	EXPECT_EQ(1, mMacro->GetEvaluateCount());
}

TEST_F(SimpleDictAdhocCommandTest, CopyActionDoesNotExpandMacrosWhenDisabled)
{
	SimpleDictParam param;
	param.mNameFormat = _T("Name");
	param.mDescriptionFormat = _T("Description");
	param.mActionType = 2;
	param.mIsExpandMacro = FALSE;
	param.mAfterCommandParam = _T("$value");

	Record record(_T("key"), _T("$Issue331TestMacro"));
	SimpleDictAdhocCommand command(param, record);

	Action* action = nullptr;
	ASSERT_TRUE(command.GetAction(HOTKEY_ATTR(0, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	action->Release();

	EXPECT_EQ(0, mMacro->GetEvaluateCount());
}
