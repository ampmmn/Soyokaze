#include "stdafx.h"
#include "commands/datetime/DateTimeCommand.h"

using launcherapp::commands::datetime::DateTimeCommand;
using launcherapp::commands::datetime::TYPE_DAY;

// 日数単位は総秒数を日数へ換算し、小数2桁で表示すること
TEST(DateTimeCommandTest, FormatsDayValue)
{
	DateTimeCommand command(CTimeSpan(1, 1, 0, 0), TYPE_DAY);
	EXPECT_EQ(_T("1.04 日"), command.GetName());
}
