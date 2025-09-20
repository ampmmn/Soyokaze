#include "pch.h"
#include "ManualEvent.h"
#include <mutex>
#include <condition_variable>
#include <chrono>

struct ManualEvent::PImpl
{
	std::mutex mMutex;
	std::condition_variable mCV;
	bool mIsSignaled{false};
};

ManualEvent::ManualEvent(bool initial)
	: in(new PImpl)
{
	in->mIsSignaled = initial;
}

ManualEvent::~ManualEvent()
{
}

void ManualEvent::Set()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mIsSignaled = true;
	in->mCV.notify_all();
}

void ManualEvent::Reset()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	in->mIsSignaled = false;
}

void ManualEvent::Wait()
{
	std::unique_lock<std::mutex> lock(in->mMutex);
	in->mCV.wait(lock, [&] { return in->mIsSignaled; });
}

bool ManualEvent::WaitFor(uint64_t milliseconds)
{
	std::unique_lock<std::mutex> lock(in->mMutex);
	return in->mCV.wait_for(lock, std::chrono::milliseconds(milliseconds),
	                           [&] { return in->mIsSignaled; });
}
