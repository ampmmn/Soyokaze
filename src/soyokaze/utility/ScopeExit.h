#pragma once

#include <utility>

namespace utility {

template<typename T>
class ScopeExit
{
public:
	ScopeExit() = delete;
	ScopeExit(const ScopeExit&) = delete;
	ScopeExit& operator = (const ScopeExit&) = delete;

	explicit ScopeExit(T&& func) : mFunc(std::move(func))
	{
	}

	~ScopeExit()
	{
		mFunc();
	}

private:
	T mFunc;
};

}
