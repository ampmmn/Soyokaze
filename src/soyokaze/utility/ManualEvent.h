#pragma once
#include <cstdint>

class ManualEvent
{
public:
	explicit ManualEvent(bool initial = false);
	~ManualEvent();

	void Set();
	void Reset();
	void Wait();
	bool WaitFor(uint64_t milliseconds);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
