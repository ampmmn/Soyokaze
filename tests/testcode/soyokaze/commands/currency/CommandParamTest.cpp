#include "stdafx.h"
#include "gtest/gtest.h"
#include "setting/Settings.h"
#include "commands/currency/CurrencyCommandParam.h"
#include "commands/currency/CurrencyConvesionCommand.h"

using namespace launcherapp::commands::currency;

TEST(CurrencyCommandParamTest, DefaultIsDisabled)
{
    CommandParam param;
    EXPECT_FALSE(param.mIsEnable);
}

TEST(CurrencyCommandParamTest, SaveAndLoad)
{
    Settings settings;
    CommandParam source;
    source.mIsEnable = true;
    source.Save(settings);

    CommandParam loaded;
    loaded.Load(settings);
    EXPECT_TRUE(loaded.mIsEnable);
}

TEST(CurrencyConvesionCommandTest, FormatsValueWithTwoDecimalPlaces)
{
    CurrencyConvesionCommand command(0.0001, _T("jpy"));

    EXPECT_EQ(_T("0.00 jpy"), command.GetName());
}
