#pragma once

namespace utility {

class TimeoutChecker
{
public:
	TimeoutChecker(uint64_t timeout) : mStart(GetTickCount64()), mTimeout(timeout) {}
	
	bool IsTimeout()
	{
		return (GetTickCount64() - mStart) >= mTimeout;
	}

	uint64_t mStart;
	uint64_t mTimeout;
	

};

}

