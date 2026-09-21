#include "stdafx.h"
#include "commands/calculator/Calculator.h"

TEST(CalculatorTest, DoesNotEvaluateWhenBothFeaturesAreDisabled)
{
	launcherapp::commands::calculator::Calculator calculator;
	calculator.UseStandardEvaluate(false);
	calculator.UseMathypadEvaluate(false);

	CString result;
	EXPECT_FALSE(calculator.Evaluate(_T("1 + 1"), result));
}
