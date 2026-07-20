#include "stdafx.h"
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class LoggerEnvironment : public ::testing::Environment
{
public:
    void SetUp() override
    {
        auto logger = spdlog::stdout_color_mt("test");
        spdlog::set_default_logger(logger);
    }
};

namespace {
[[maybe_unused]]
const auto* logger_env = ::testing::AddGlobalTestEnvironment(new LoggerEnvironment());
}

