#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/output_preview/OutputPreviewCommand.h"
#include "actions/core/Action.h"
#include "hotkey/CommandHotKeyAttribute.h"
#include "commands/output_preview/OutputPreviewRuntime.h"
#include "matcher/PartialMatchPattern.h"

using launcherapp::commands::output_preview::OutputPreviewCommand;
using launcherapp::commands::output_preview::OutputPreviewParam;

TEST(OutputPreviewCommandTest, ExposesConfiguredNameAndDescription)
{
	OutputPreviewCommand command;
	OutputPreviewParam param;
	param.mName = _T("preview");
	param.mDescription = _T("output description");
	command.SetParam(param);

	EXPECT_EQ(command.GetName(), _T("preview"));
	EXPECT_EQ(command.GetDescription(), _T("output description"));
	EXPECT_EQ(command.GetType(), _T("OutputPreview"));
	EXPECT_EQ(command.GetTypeDisplayName(), _T("実行結果確認コマンド"));
	EXPECT_FALSE(command.IsAcceptArguments());
}

TEST(OutputPreviewCommandTest, ProvidesExpectedEnterActions)
{
	OutputPreviewCommand command;
	OutputPreviewParam param;
	param.mName = _T("preview");
	param.mFilePath = _T("C:\\Tools\\tool.exe");
	command.SetParam(param);

	const UINT modifiersList[] = { 0u, MOD_CONTROL, MOD_ALT };
	for (UINT modifiers : modifiersList) {
		launcherapp::actions::core::Action* action = nullptr;
		ASSERT_TRUE(command.GetAction(HOTKEY_ATTR(modifiers, VK_RETURN), &action));
		ASSERT_NE(nullptr, action);
		action->Release();
	}

	CommandHotKeyAttribute hotkey;
	EXPECT_FALSE(command.GetHotKeyAttribute(hotkey));
}

TEST(OutputPreviewCommandTest, HidesActiveCommandWhileItsResultIsDisplayed)
{
	auto runtime = launcherapp::commands::output_preview::OutputPreviewRuntime::Get();
	ASSERT_TRUE(runtime->TryBegin(_T("preview-session"), _T("preview"), _T("preview --argument")));
	ASSERT_TRUE(runtime->Publish(_T("preview-session"), _T("result"), _T(""), 0, _T("")));

	OutputPreviewCommand command;
	OutputPreviewParam param;
	param.mName = _T("preview");
	command.SetParam(param);

	PartialMatchPattern* pattern = PartialMatchPattern::Create();
	ASSERT_NE(nullptr, pattern);
	pattern->SetWholeText(_T("preview --argument"));
	EXPECT_EQ(command.Match(pattern), Pattern::Mismatch);
	pattern->SetWholeText(_T("preview"));
	EXPECT_NE(command.Match(pattern), Pattern::Mismatch);
	pattern->Release();

	runtime->Close(_T("preview-session"));
}
