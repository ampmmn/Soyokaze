#include "stdafx.h"
#include "NormalPriviledgeRequestBuffer.h"

TEST(NormalPriviledgeRequestBuffer, returnsFalseUntilRequestIsComplete)
{
	NormalPriviledgeRequestBuffer buffer;
	std::string request;

	buffer.Append("{\"command\":", 11);
	EXPECT_FALSE(buffer.TryPopRequest(request));
	EXPECT_TRUE(buffer.HasPendingData());

	buffer.Append("\"test\"}\n", 8);
	EXPECT_TRUE(buffer.TryPopRequest(request));
	EXPECT_EQ("{\"command\":\"test\"}", request);
	EXPECT_FALSE(buffer.TryPopRequest(request));
	EXPECT_FALSE(buffer.HasPendingData());
}

TEST(NormalPriviledgeRequestBuffer, keepsFollowingRequests)
{
	NormalPriviledgeRequestBuffer buffer;
	std::string request;
	const std::string requests = "first\nsecond\n";

	buffer.Append(requests.data(), requests.size());
	ASSERT_TRUE(buffer.TryPopRequest(request));
	EXPECT_EQ("first", request);
	ASSERT_TRUE(buffer.TryPopRequest(request));
	EXPECT_EQ("second", request);
	EXPECT_FALSE(buffer.TryPopRequest(request));
	EXPECT_FALSE(buffer.HasPendingData());
}

TEST(NormalPriviledgeRequestBuffer, discardsNullTerminatorEvenWhenSplitFromNewline)
{
	NormalPriviledgeRequestBuffer buffer;
	std::string request;

	buffer.Append("first\n", 6);
	ASSERT_TRUE(buffer.TryPopRequest(request));
	EXPECT_EQ("first", request);

	buffer.Append("\0second\n\0", 9);
	ASSERT_TRUE(buffer.TryPopRequest(request));
	EXPECT_EQ("second", request);
	EXPECT_FALSE(buffer.TryPopRequest(request));
}
