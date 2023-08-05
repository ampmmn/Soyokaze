#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/ExitCommand.h"


TEST(test, test)
{
	ExitCommand* command = new ExitCommand();

	CString str = command->GetDescription();
	EXPECT_EQ(_T("ÅyèIóπÅz"), str);
}
