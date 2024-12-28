#pragma once

#include "commands/core/CommandParameter.h"

namespace launcherapp {
namespace commands {
namespace core {

class CommandQueryRequest
{
	using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;

public:
	CommandQueryRequest() : mHwnd(nullptr), mMsg(0) {}
	CommandQueryRequest(const CommandParameterBuilder* param, HWND hwndNotify, UINT notifyMsg) :
		mParam(param->Clone_()), mHwnd(hwndNotify), mMsg(notifyMsg)
	{
	}
	CommandQueryRequest(const CommandQueryRequest& rhs) : 
		mParam(rhs.mParam->Clone_()), mHwnd(rhs.mHwnd), mMsg(rhs.mMsg)
	{
	}
	~CommandQueryRequest() {
		if (mParam) {
			mParam->Release();
			mParam = nullptr;
		}
	}

	CommandQueryRequest& operator = (const CommandQueryRequest& rhs)
	{
		if (&rhs != this) {
			mParam = rhs.mParam->Clone_();
			mHwnd = rhs.mHwnd;
			mMsg = rhs.mMsg;
		}
		return *this;
	}

	const CommandParameterBuilder* GetCommandParameter() { return mParam; }
	HWND GetNotifyWindow() { return mHwnd; }
	UINT GetNotifyMessage() { return mMsg; }

private:
	CommandParameterBuilder* mParam = nullptr;
	HWND mHwnd = nullptr;
	UINT mMsg = 0;
};



}
}
}

