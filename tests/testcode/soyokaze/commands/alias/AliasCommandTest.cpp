#include "stdafx.h"
#include "commands/alias/AliasCommand.h"
#include "commands/alias/AliasCommandParam.h"
#include "core/IFIDDefine.h"
#include "utility/RefPtr.h"

using launcherapp::commands::alias::AliasCommand;
using launcherapp::commands::alias::CommandParam;
using launcherapp::core::SelectionBehavior;

TEST(AliasCommandTest, PasteOnlyKeepsMainWindowOpen)
{
	AliasCommand command;
	CommandParam param;
	param.mIsPasteOnly = 1;
	command.SetParam(param);

	RefPtr<SelectionBehavior> behavior;
	ASSERT_TRUE(command.QueryInterface(IFID_SELECTIONBEHAVIOR, (void**)&behavior));
	EXPECT_EQ(SelectionBehavior::CLOSEWINDOW_NOCLOSE, behavior->GetCloseWindowPolicy(0));
}

TEST(AliasCommandTest, ExecutingAliasClosesMainWindow)
{
	AliasCommand command;
	CommandParam param;
	param.mIsPasteOnly = 0;
	command.SetParam(param);

	RefPtr<SelectionBehavior> behavior;
	ASSERT_TRUE(command.QueryInterface(IFID_SELECTIONBEHAVIOR, (void**)&behavior));
	EXPECT_EQ(SelectionBehavior::CLOSEWINDOW_ASYNC, behavior->GetCloseWindowPolicy(0));
}
