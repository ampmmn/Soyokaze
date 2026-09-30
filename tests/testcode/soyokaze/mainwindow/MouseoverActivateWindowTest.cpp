#include "stdafx.h"
#include "gtest/gtest.h"
#include "features/main/MouseoverActivateWindow.h"

TEST(MouseoverActivateWindowTest, MarginScalesWithDpi)
{
	EXPECT_EQ(MouseoverActivateState::GetMouseoverActivateMarginForDpi(96), 12);
	EXPECT_EQ(MouseoverActivateState::GetMouseoverActivateMarginForDpi(120), 15);
	EXPECT_EQ(MouseoverActivateState::GetMouseoverActivateMarginForDpi(144), 18);
	EXPECT_EQ(MouseoverActivateState::GetMouseoverActivateMarginForDpi(192), 24);
	EXPECT_EQ(MouseoverActivateState::GetMouseoverActivateMarginForDpi(0), 12);
}

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
