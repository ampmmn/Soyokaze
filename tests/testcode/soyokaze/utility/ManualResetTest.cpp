#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/ManualEvent.h"
#include <thread>
#include <atomic>

TEST(ManualEventTest, InitialStateFalse_WaitBlocksUntilSet)
{
    ManualEvent ev(false);
    std::atomic<bool> waited{false};

    std::thread th([&] {
        ev.Wait();
        waited = true;
    });

    // Wait a bit to ensure thread is blocked
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    EXPECT_FALSE(waited);

    ev.Set();
    th.join();
    EXPECT_TRUE(waited);
}

TEST(ManualEventTest, InitialStateTrue_WaitReturnsImmediately)
{
    ManualEvent ev(true);
    std::atomic<bool> waited{false};

    std::thread th([&] {
        ev.Wait();
        waited = true;
    });

    th.join();
    EXPECT_TRUE(waited);
}

TEST(ManualEventTest, ResetBlocksWaitUntilSetAgain)
{
    ManualEvent ev(true);
    ev.Reset();

    std::atomic<bool> waited{false};
    std::thread th([&] {
        ev.Wait();
        waited = true;
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    EXPECT_FALSE(waited);

    ev.Set();
    th.join();
    EXPECT_TRUE(waited);
}

TEST(ManualEventTest, WaitForTimeout)
{
    ManualEvent ev(false);
    auto start = std::chrono::steady_clock::now();
    bool result = ev.WaitFor(100);
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    EXPECT_FALSE(result);
    EXPECT_GE(elapsed, 90); // Allow some margin for timing
}

TEST(ManualEventTest, WaitForReturnsTrueIfSet)
{
    ManualEvent ev(false);

    std::thread th([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        ev.Set();
    });

    bool result = ev.WaitFor(500);
    th.join();
    EXPECT_TRUE(result);
}

TEST(ManualEventTest, WaitFor0)
{
    ManualEvent ev(false);
    bool result = ev.WaitFor(0);
		EXPECT_FALSE(result);
		ev.Set();
    result = ev.WaitFor(0);
		EXPECT_TRUE(result);
}

