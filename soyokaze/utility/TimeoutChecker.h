#pragma once

namespace utility {

class TimeoutChecker
{
public:
	TimeoutChecker(DWORD timeout) : mStart(GetTickCount()), mTimeout(timeout) {}
	
	bool IsTimeout()
	{
		return (GetTickCount() - mStart) >= mTimeout;
	}

	DWORD mStart;
	DWORD mTimeout;
	

};

}

