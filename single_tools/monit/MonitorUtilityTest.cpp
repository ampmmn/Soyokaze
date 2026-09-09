#include "MonitorUtility.h"

#include <gtest/gtest.h>

TEST(MonitorUtilityTest, ParsesInputSourceAlias)
{
	unsigned int value = 0;
	EXPECT_TRUE(ParseInputSourceValue("HdMi1", value));
	EXPECT_EQ(value, 17U);
}

TEST(MonitorUtilityTest, ParsesVendorInputSource)
{
	unsigned int value = 0;
	EXPECT_TRUE(ParseInputSourceValue("255", value));
	EXPECT_EQ(value, 255U);
	EXPECT_FALSE(ParseInputSourceValue("256", value));
}

TEST(MonitorUtilityTest, ParsesCapabilities)
{
	const auto sources = ParseInputSources("(prot(monitor) vcp(10 12 60(0f 11 18 7f) 62))");
	ASSERT_EQ(sources.size(), 4U);
	EXPECT_EQ(sources[0].id, 15U);
	EXPECT_EQ(sources[0].displayName, "DisplayPort1");
	EXPECT_EQ(sources[1].displayName, "HDMI1");
	EXPECT_EQ(sources[2].displayName, "24");
	EXPECT_EQ(sources[3].displayName, "127");
}

TEST(MonitorUtilityTest, ConvertsBrightness)
{
	EXPECT_EQ(NormalizeBrightness(50, 10, 110), 40);
	unsigned int deviceValue = 0;
	EXPECT_TRUE(DenormalizeBrightness(40, 10, 110, deviceValue));
	EXPECT_EQ(deviceValue, 50U);
	EXPECT_FALSE(DenormalizeBrightness(101, 10, 110, deviceValue));
}
