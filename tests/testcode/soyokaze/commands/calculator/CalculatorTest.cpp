#include "stdafx.h"
#include "commands/calculator/Calculator.h"

TEST(CalculatorTest, RejectsStringExpression)
{
	launcherapp::commands::calculator::Calculator calculator;
	CString result;

	EXPECT_FALSE(calculator.Evaluate(_T("\"text\""), result));
}

TEST(CalculatorTest, RejectsInvalidExpression)
{
	launcherapp::commands::calculator::Calculator calculator;
	CString result;

	EXPECT_FALSE(calculator.Evaluate(_T("1=2"), result));
}
