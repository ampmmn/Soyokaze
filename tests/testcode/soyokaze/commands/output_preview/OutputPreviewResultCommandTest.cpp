#include "stdafx.h"
#include "gtest/gtest.h"
#include "actions/core/Action.h"
#include "commands/core/PreviewProvider.h"
#include "commands/output_preview/OutputPreviewResultCommand.h"
#include "core/IFIDDefine.h"

using launcherapp::commands::output_preview::OutputPreviewResultCommand;
using launcherapp::commands::output_preview::OutputPreviewRuntime;

TEST(OutputPreviewResultCommandTest, ProvidesPreviewAndResultCommandInterfaces)
{
	OutputPreviewRuntime::Result result;
	result.mName = _T("preview-session");
	result.mCommandName = _T("preview");
	result.mText = _T("output text");
	OutputPreviewResultCommand command(result);
	EXPECT_EQ(command.GetName(), _T("preview"));

	launcherapp::core::PreviewProvider* previewProvider = nullptr;
	ASSERT_TRUE(command.QueryInterface(IFID_PREVIEWPROVIDER, (void**)&previewProvider));
	ASSERT_NE(nullptr, previewProvider);
	previewProvider->Release();

	OutputPreviewResultCommand* resultCommand = nullptr;
	ASSERT_TRUE(command.QueryInterface(IFID_OUTPUTPREVIEWRESULTCOMMAND, (void**)&resultCommand));
	ASSERT_NE(nullptr, resultCommand);
	resultCommand->Release();
}

TEST(OutputPreviewResultCommandTest, DisplaysCloseAndCopyAndCloseActions)
{
	OutputPreviewRuntime::Result result;
	result.mName = _T("preview-session");
	result.mCommandName = _T("preview");
	result.mText = _T("output text");
	OutputPreviewResultCommand command(result);

	launcherapp::actions::core::Action* action = nullptr;
	ASSERT_TRUE(command.GetAction(HOTKEY_ATTR(0, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	EXPECT_EQ(action->GetDisplayName(), _T("プレビューを閉じる"));
	action->Release();

	action = nullptr;
	ASSERT_TRUE(command.GetAction(HOTKEY_ATTR(MOD_SHIFT, VK_RETURN), &action));
	ASSERT_NE(nullptr, action);
	EXPECT_EQ(action->GetDisplayName(), _T("クリップボードにコピーして閉じる"));
	action->Release();
}
