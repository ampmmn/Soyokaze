#include "stdafx.h"
#include "gtest/gtest.h"
#include "processproxy/NormalPriviledgeProcessProxy.h"

using launcherapp::processproxy::detail::NormalPriviledgeAgentAction;
using launcherapp::processproxy::detail::NormalPriviledgeAgentConnectionState;
using launcherapp::processproxy::detail::NormalPriviledgeAgentStatus;

TEST(NormalPriviledgeProcessProxyTest, StartsWhenConnectionIsNotStarted)
{
	NormalPriviledgeAgentConnectionState state;

	EXPECT_EQ(NormalPriviledgeAgentAction::Start,
		state.Evaluate(false, NormalPriviledgeAgentStatus::NotResponding));
	EXPECT_FALSE(state.IsStarted());
}

TEST(NormalPriviledgeProcessProxyTest, ReusesResponsiveAgent)
{
	NormalPriviledgeAgentConnectionState state;
	state.MarkStarted();

	EXPECT_EQ(NormalPriviledgeAgentAction::Reuse,
		state.Evaluate(true, NormalPriviledgeAgentStatus::Available));
	EXPECT_TRUE(state.IsStarted());
}

TEST(NormalPriviledgeProcessProxyTest, KeepsBusyAgentForLaterRetry)
{
	NormalPriviledgeAgentConnectionState state;
	state.MarkStarted();

	EXPECT_EQ(NormalPriviledgeAgentAction::Wait,
		state.Evaluate(true, NormalPriviledgeAgentStatus::Busy));
	EXPECT_TRUE(state.IsStarted());
	EXPECT_EQ(NormalPriviledgeAgentAction::Reuse,
		state.Evaluate(true, NormalPriviledgeAgentStatus::Available));
}

TEST(NormalPriviledgeProcessProxyTest, RestartsWhenServerWindowDisappears)
{
	NormalPriviledgeAgentConnectionState state;
	state.MarkStarted();

	EXPECT_EQ(NormalPriviledgeAgentAction::Start,
		state.Evaluate(false, NormalPriviledgeAgentStatus::NotResponding));
	EXPECT_FALSE(state.IsStarted());
	EXPECT_EQ(NormalPriviledgeAgentAction::Start,
		state.Evaluate(false, NormalPriviledgeAgentStatus::NotResponding));
}

TEST(NormalPriviledgeProcessProxyTest, RestartsWhenAgentStopsResponding)
{
	NormalPriviledgeAgentConnectionState state;
	state.MarkStarted();

	EXPECT_EQ(NormalPriviledgeAgentAction::Start,
		state.Evaluate(true, NormalPriviledgeAgentStatus::NotResponding));
	EXPECT_FALSE(state.IsStarted());
}
