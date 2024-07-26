#pragma once

#include "commands/core/CommandParameter.h"

namespace launcherapp {
namespace commands {
namespace core {

class CommandQueryRequest
{
	using CommandParameter = launcherapp::core::CommandParameter;

public:
	CommandQueryRequest() : mHwnd(nullptr), mMsg(0) {}
	CommandQueryRequest(const CommandParameter& param, HWND hwndNotify, UINT notifyMsg) :
		mParam(param), mHwnd(hwndNotify), mMsg(notifyMsg)
	{
	}
	CommandQueryRequest(const CommandQueryRequest& rhs) : 
		mParam(rhs.mParam), mHwnd(rhs.mHwnd), mMsg(rhs.mMsg)
	{
	}

	CommandQueryRequest& operator = (const CommandQueryRequest& rhs)
	{
		if (&rhs != this) {
			mParam = rhs.mParam;
			mHwnd = rhs.mHwnd;
			mMsg = rhs.mMsg;
		}
		return *this;
	}

	const CommandParameter& GetCommandParameter() { return mParam; }
	HWND GetNotifyWindow() { return mHwnd; }
	UINT GetNotifyMessage() { return mMsg; }

private:
	CommandParameter mParam;
	HWND mHwnd;
	UINT mMsg;
};



}
}
}

