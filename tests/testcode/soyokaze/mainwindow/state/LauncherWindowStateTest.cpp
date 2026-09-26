#include "stdafx.h"
#include "gtest/gtest.h"
#include "mainwindow/state/MainWindowHiddenState.h"
#include "mainwindow/state/MainWindowSearchingState.h"
#include "mainwindow/state/MainWindowParamSearchingState.h"
#include "mainwindow/state/LauncherWindowStateContextIF.h"
#include "mainwindow/state/MainWindowIdleState.h"

namespace {

class DummyContext : public launcherapp::mainwindow::state::LauncherWindowStateContextIF
{
public:
	void ChangeState(std::unique_ptr<launcherapp::mainwindow::state::LauncherWindowState> state) override
	{
		if (mState) {
			mState->OnExit();
		}
		mState = std::move(state);
		if (mState) {
			mState->OnEnter();
		}
	}

	void ShowWindowFromState() override
	{
		mShowCount++;
		mIsWindowVisible = true;
		mIsWindowActive = true;
	}

	void ActivateVisibleWindow() override
	{
		mActivateCount++;
		mIsWindowVisible = true;
		mIsWindowActive = true;
	}

	void HideWindowFromState() override
	{
		mIsWindowVisible = false;
		mIsWindowActive = false;
		mHideCount++;
	}

	void ClearContent() override
	{
		mHasKeyword = false;
		mClearCount++;
	}

	void SetFocusToEdit() override
	{
		mFocusCount++;
	}

	void HandleTextChanged() override
	{
	}

	void UpdateInputState() override
	{
	}

	void HandleQueryCompleted(launcherapp::commands::core::CommandQueryResult* result) override
	{
		UNREFERENCED_PARAMETER(result);
	}

	bool IsCandidateListEmpty() const override
	{
		return mIsCandidateListEmpty;
	}

	void OffsetCandidateSelection(int offset, bool isLoop) override
	{
		mOffset = offset;
		mIsLoop = isLoop;
	}

	void UpdateCurrentCandidate() override
	{
		mUpdateCandidateCount++;
	}

	int GetCandidateCountInPage() override
	{
		return mCandidateCountInPage;
	}

	void Complement() override
	{
		mComplementCount++;
	}

	void ReflectCurrentCandidate() override
	{
	}

	void SelectCandidate(int index) override
	{
		UNREFERENCED_PARAMETER(index);
	}

	void ExecuteCurrentCommand() override
	{
		mExecuteCount++;
		mIsWindowVisible = false;
		mIsWindowActive = false;
	}

	bool HasKeyword() const override
	{
		return mHasKeyword;
	}

	bool IsWindowVisibleFromState() const override
	{
		return mIsWindowVisible;
	}

	bool IsWindowActive() const override
	{
		return mIsWindowActive;
	}

	bool IsShowToggleEnabled() const override
	{
		return mIsShowToggle;
	}

	bool CanStartParamSearching() override
	{
		return mCanStartParamSearching;
	}

	void RequestParamSearching() override
	{
		mState = std::make_unique<launcherapp::mainwindow::state::ParamSearchingState>(this);
	}

	void UpdateExtraCandidates() override
	{
	}

	void HideExtraCandidates() override
	{
		mHideExtraCandidatesCount++;
	}

	void OffsetExtraCandidateSelection(int offset) override
	{
		UNREFERENCED_PARAMETER(offset);
	}

	bool IsExtraCandidateListEmpty() const override
	{
		return mIsExtraCandidateListEmpty;
	}

	void ResolveExtraCandidate() override
	{
	}

	std::unique_ptr<launcherapp::mainwindow::state::LauncherWindowState> mState;
	bool mHasKeyword{false};
	bool mIsWindowVisible{false};
	bool mIsWindowActive{false};
	bool mIsShowToggle{false};
	bool mCanStartParamSearching{false};
	bool mIsCandidateListEmpty{false};
	bool mIsLoop{false};
	int mOffset{0};
	int mCandidateCountInPage{5};
	int mUpdateCandidateCount{0};
	int mComplementCount{0};
	int mShowCount{0};
	int mActivateCount{0};
	int mHideCount{0};
	int mClearCount{0};
	int mFocusCount{0};
	int mExecuteCount{0};
	int mHideExtraCandidatesCount{0};
	bool mIsExtraCandidateListEmpty{true};
};

}

TEST(LauncherWindowStateTest, HiddenState_ShowRequested_TransitionsToIdleState)
{
	DummyContext context;
	launcherapp::mainwindow::state::HiddenState state(&context);

	state.OnShowRequested(false);

	EXPECT_EQ(context.mShowCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::IdleState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, HiddenState_Enter_HidesVisibleWindow)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::HiddenState state(&context);

	state.OnEnter();

	EXPECT_EQ(context.mHideCount, 1);
	EXPECT_FALSE(context.mIsWindowVisible);
}

TEST(LauncherWindowStateTest, HiddenState_HideRequested_HidesVisibleWindow)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::HiddenState state(&context);

	state.OnHideRequested();

	EXPECT_EQ(context.mHideCount, 1);
	EXPECT_FALSE(context.mIsWindowVisible);
}

TEST(LauncherWindowStateTest, HiddenState_HideRequested_DoesNotHideHiddenWindowAgain)
{
	DummyContext context;
	launcherapp::mainwindow::state::HiddenState state(&context);

	state.OnHideRequested();

	EXPECT_EQ(context.mHideCount, 0);
}

TEST(LauncherWindowStateTest, IdleState_ShowRequestedWhenInactive_ActivatesWindow)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::IdleState state(&context);

	state.OnShowRequested(false);

	EXPECT_EQ(context.mActivateCount, 1);
	EXPECT_EQ(context.mHideCount, 0);
}

TEST(LauncherWindowStateTest, IdleState_ActivateNotificationDoesNotShowOrHideWindow)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::IdleState state(&context);

	state.OnActivate();

	EXPECT_EQ(context.mShowCount, 0);
	EXPECT_EQ(context.mActivateCount, 0);
	EXPECT_EQ(context.mHideCount, 0);
}

TEST(LauncherWindowStateTest, IdleState_HideRequested_TransitionsToHiddenState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mState = std::make_unique<launcherapp::mainwindow::state::IdleState>(&context);

	context.mState->OnHideRequested();

	EXPECT_EQ(context.mHideCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::HiddenState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_HideRequested_TransitionsToHiddenState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mState = std::make_unique<launcherapp::mainwindow::state::SearchingState>(&context);

	context.mState->OnHideRequested();

	EXPECT_EQ(context.mHideCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::HiddenState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, ParamSearchingState_HideRequested_TransitionsToHiddenState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mState = std::make_unique<launcherapp::mainwindow::state::ParamSearchingState>(&context);

	context.mState->OnHideRequested();

	EXPECT_EQ(context.mHideCount, 1);
	EXPECT_EQ(context.mHideExtraCandidatesCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::HiddenState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, IdleState_ShowRequestedWhenActiveAndToggleEnabled_HidesWindow)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mIsWindowActive = true;
	context.mIsShowToggle = true;
	launcherapp::mainwindow::state::IdleState state(&context);

	state.OnShowRequested(false);

	EXPECT_EQ(context.mActivateCount, 0);
	EXPECT_EQ(context.mHideCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::HiddenState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, IdleState_ShowRequestedWhenWindowIsHidden_ShowsWindow)
{
	DummyContext context;
	launcherapp::mainwindow::state::IdleState state(&context);

	state.OnShowRequested(false);

	EXPECT_EQ(context.mShowCount, 1);
	EXPECT_EQ(context.mActivateCount, 0);
}

TEST(LauncherWindowStateTest, ParamSearchingState_ShowRequestedWhenWindowIsHidden_ShowsWindow)
{
	DummyContext context;
	launcherapp::mainwindow::state::ParamSearchingState state(&context);

	state.OnShowRequested(false);

	EXPECT_EQ(context.mShowCount, 1);
	EXPECT_EQ(context.mActivateCount, 0);
}

TEST(LauncherWindowStateTest, IdleState_ForceShowRequested_ShowsWindow)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mIsWindowActive = true;
	context.mIsShowToggle = true;
	launcherapp::mainwindow::state::IdleState state(&context);

	state.OnShowRequested(true);

	EXPECT_EQ(context.mShowCount, 1);
	EXPECT_EQ(context.mHideCount, 0);
}

TEST(LauncherWindowStateTest, ParamSearchingState_Deactivate_HidesExtraCandidatesWithoutChangingState)
{
	DummyContext context;
	context.mState = std::make_unique<launcherapp::mainwindow::state::ParamSearchingState>(&context);

	context.mState->OnDeactivate();

	EXPECT_EQ(context.mHideExtraCandidatesCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::ParamSearchingState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, ParamSearchingState_TextChangedWithoutExtraCandidates_TransitionsToSearchingState)
{
	DummyContext context;
	context.mHasKeyword = true;
	context.mCanStartParamSearching = true;
	context.mIsExtraCandidateListEmpty = true;
	context.mState = std::make_unique<launcherapp::mainwindow::state::ParamSearchingState>(&context);

	context.mState->OnTextChanged();

	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::SearchingState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, IdleState_TextChangedWithKeyword_TransitionsToSearchingState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mHasKeyword = true;
	launcherapp::mainwindow::state::IdleState state(&context);

	state.OnTextChanged();

	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::SearchingState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_Cancel_ClearsAndTransitionsToIdleState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mHasKeyword = true;
	launcherapp::mainwindow::state::SearchingState state(&context);

	state.OnCancel();

	EXPECT_EQ(context.mClearCount, 1);
	EXPECT_EQ(context.mFocusCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::IdleState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_TextChangedWithParameter_RemainsSearchingStateUntilQueryCompletes)
{
	DummyContext context;
	context.mHasKeyword = true;
	context.mCanStartParamSearching = true;
	launcherapp::mainwindow::state::SearchingState state(&context);

	state.OnTextChanged();

	EXPECT_EQ(context.mState, nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_QueryCompletedWithParameter_TransitionsToParamSearchingState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mHasKeyword = true;
	context.mCanStartParamSearching = true;
	launcherapp::mainwindow::state::SearchingState state(&context);

	state.OnQueryCompleted(nullptr);

	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::ParamSearchingState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_QueryCompletedWithoutParamSearching_DoesNotTransitionToParamSearchingState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mHasKeyword = true;
	context.mCanStartParamSearching = false;
	launcherapp::mainwindow::state::SearchingState state(&context);

	state.OnQueryCompleted(nullptr);

	EXPECT_EQ(context.mState, nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_QueryCompletedAfterExtraCandidateResolve_DoesNotTransitionToParamSearchingState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mHasKeyword = true;
	context.mCanStartParamSearching = true;
	launcherapp::mainwindow::state::SearchingState state(&context, false);

	state.OnQueryCompleted(nullptr);

	EXPECT_EQ(context.mState, nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_TextChangedAfterExtraCandidateResolve_AllowsParamSearchingAgain)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mHasKeyword = true;
	context.mCanStartParamSearching = true;
	launcherapp::mainwindow::state::SearchingState state(&context, false);

	state.OnTextChanged();
	state.OnQueryCompleted(nullptr);

	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::ParamSearchingState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_TextChangedWithoutParameterSearch_RemainsSearchingState)
{
	DummyContext context;
	context.mHasKeyword = true;
	context.mCanStartParamSearching = false;
	launcherapp::mainwindow::state::SearchingState state(&context);

	state.OnTextChanged();

	EXPECT_EQ(context.mState, nullptr);
}

TEST(LauncherWindowStateTest, SearchingState_ExecuteWhenClosed_TransitionsToHiddenState)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	context.mHasKeyword = true;
	launcherapp::mainwindow::state::SearchingState state(&context);

	state.OnExecuteRequested();

	EXPECT_EQ(context.mExecuteCount, 1);
	EXPECT_NE(dynamic_cast<launcherapp::mainwindow::state::HiddenState*>(context.mState.get()), nullptr);
}

TEST(LauncherWindowStateTest, IdleState_Enter_ExecutesCommand)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::IdleState state(&context);

	EXPECT_TRUE(state.OnKeyInput(VK_RETURN));
	EXPECT_EQ(context.mExecuteCount, 1);
}

TEST(LauncherWindowStateTest, IdleState_DoesNotHandleCandidateKeys)
{
	DummyContext context;
	launcherapp::mainwindow::state::IdleState state(&context);

	EXPECT_FALSE(state.OnKeyInput(VK_UP));
	EXPECT_FALSE(state.OnKeyInput(VK_TAB));
	EXPECT_FALSE(state.OnKeyInput(VK_NEXT));
}

TEST(LauncherWindowStateTest, SearchingState_ArrowKey_UpdatesCandidate)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::SearchingState state(&context);

	EXPECT_TRUE(state.OnKeyInput(VK_UP));
	EXPECT_EQ(context.mOffset, -1);
	EXPECT_TRUE(context.mIsLoop);
	EXPECT_EQ(context.mUpdateCandidateCount, 1);
}

TEST(LauncherWindowStateTest, SearchingState_Tab_ComplementsKeyword)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::SearchingState state(&context);

	EXPECT_TRUE(state.OnKeyInput(VK_TAB));
	EXPECT_EQ(context.mComplementCount, 1);
}

TEST(LauncherWindowStateTest, SearchingState_PageKey_OffsetsByPageSize)
{
	DummyContext context;
	context.mIsWindowVisible = true;
	launcherapp::mainwindow::state::SearchingState state(&context);

	EXPECT_TRUE(state.OnKeyInput(VK_PRIOR));
	EXPECT_EQ(context.mOffset, -5);
	EXPECT_FALSE(context.mIsLoop);
}
