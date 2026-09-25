#include "stdafx.h"
#include "gtest/gtest.h"
#include "mainwindow/MouseoverActivateWindow.h"

TEST(MouseoverActivateWindowTest, CursorEnteringAndLeavingActivatesAndDeactivates)
{
	MouseoverActivateState state;

	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(false, false), MouseoverActivateState::Action::Deactivate);
	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::Activate);
}

TEST(MouseoverActivateWindowTest, DraggingAcrossWindowBoundaryDoesNotSwitchActivation)
{
	MouseoverActivateState state;

	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(true, true), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(false, true), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(false, false), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(false, false), MouseoverActivateState::Action::Deactivate);
	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::Activate);
}

TEST(MouseoverActivateWindowTest, DraggingIntoWindowDefersActivationUntilAfterButtonRelease)
{
	MouseoverActivateState state;

	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(false, false), MouseoverActivateState::Action::Deactivate);
	EXPECT_EQ(state.Update(false, true), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(true, true), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::Activate);
}

TEST(MouseoverActivateWindowTest, ResetClearsDragAndCursorState)
{
	MouseoverActivateState state;

	state.Update(true, false);
	state.Update(false, true);
	state.Reset();

	EXPECT_EQ(state.Update(true, false), MouseoverActivateState::Action::None);
	EXPECT_EQ(state.Update(false, false), MouseoverActivateState::Action::Deactivate);
}
