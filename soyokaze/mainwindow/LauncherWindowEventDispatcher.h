#pragma once

#include "mainwindow/LauncherWindowEventListenerIF.h"
#include <functional>

class LauncherWindowEventDispatcher
{
public:
	using Listener = LauncherWindowEventListenerIF;

private:
	LauncherWindowEventDispatcher();
	~LauncherWindowEventDispatcher();

public:
	static LauncherWindowEventDispatcher* Get();

	void AddListener(Listener* listener);
	void RemoveListener(Listener* listener);
	void Dispatch(std::function<void(Listener*)> callback);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

