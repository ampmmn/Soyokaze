#pragma once

#include "core/LauncherEventListenerIF.h"
#include <functional>

class LauncherEventDispatcher
{
public:
	using Listener = LauncherEventListenerIF;

private:
	LauncherEventDispatcher();
	~LauncherEventDispatcher();

public:
	static LauncherEventDispatcher* Get();

	void AddListener(Listener* listener);
	void RemoveListener(Listener* listener);
	void Dispatch(std::function<void(Listener*)> callback);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
