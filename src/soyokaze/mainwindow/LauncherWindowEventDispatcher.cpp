#include "pch.h"
#include "LauncherWindowEventDispatcher.h"
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using Listener = LauncherWindowEventDispatcher::Listener;

struct LauncherWindowEventDispatcher::PImpl
{
	std::set<Listener*> mListeners;
};

LauncherWindowEventDispatcher::LauncherWindowEventDispatcher() : in(new PImpl)
{
}

LauncherWindowEventDispatcher::~LauncherWindowEventDispatcher()
{
}

LauncherWindowEventDispatcher* LauncherWindowEventDispatcher::Get()
{
	static LauncherWindowEventDispatcher inst;
	return &inst;
}

void LauncherWindowEventDispatcher::AddListener(Listener* listener)
{
	in->mListeners.insert(listener);
}

void LauncherWindowEventDispatcher::RemoveListener(Listener* listener)
{
	in->mListeners.erase(listener);
}

void LauncherWindowEventDispatcher::Dispatch(std::function<void(Listener*)> callback)
{
	// $BDLCN@h$G%j%9%J!<2r=|$9$k2DG=@-$,$"$k$?$a!"%3%T!<$r<h$k(B
	auto listeners = in->mListeners;

	for (auto& listener : listeners) {
		callback(listener);
	}
}

