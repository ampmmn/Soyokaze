#pragma once

#include "utility/ManualEvent.h"

// スコープを抜けるときにResetEventを解除する(SetEventを呼ぶ)
class ScopedResetEvent
{
public:
	ScopedResetEvent(ManualEvent& event, bool isReset) : mEvent(event)
 	{
		if (isReset) {
			event.Reset();
		}
	}
	~ScopedResetEvent() {
		mEvent.Set();
	}

	ScopedResetEvent(const ScopedResetEvent&) = delete;
	ScopedResetEvent& operator = (const ScopedResetEvent&) = delete;
	
	ManualEvent& mEvent;
};
