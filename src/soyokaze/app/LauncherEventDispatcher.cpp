#include "pch.h"
#include "LauncherEventDispatcher.h"
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Listener = LauncherEventDispatcher::Listener;

struct LauncherEventDispatcher::PImpl
{
	std::set<Listener*> mListeners;
};

LauncherEventDispatcher::LauncherEventDispatcher() : in(new PImpl)
{
}

LauncherEventDispatcher::~LauncherEventDispatcher()
{
}

LauncherEventDispatcher* LauncherEventDispatcher::Get()
{
	static LauncherEventDispatcher inst;
	return &inst;
}

void LauncherEventDispatcher::AddListener(Listener* listener)
{
	in->mListeners.insert(listener);
}

void LauncherEventDispatcher::RemoveListener(Listener* listener)
{
	in->mListeners.erase(listener);
}

void LauncherEventDispatcher::Dispatch(std::function<void(Listener*)> callback)
{
	for (auto& listener : in->mListeners) {
		callback(listener);
	}
}
