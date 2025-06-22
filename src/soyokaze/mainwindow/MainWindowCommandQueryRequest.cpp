#include "pch.h"
#include "MainWindowCommandQueryRequest.h"

MainWindowCommandQueryRequest::MainWindowCommandQueryRequest(
	const CString& keyword,
	HWND hwndNotify,
	UINT notifyMsg
) :
	mKeyword(keyword), mHwnd(hwndNotify), mMsg(notifyMsg)
{
}

MainWindowCommandQueryRequest::~MainWindowCommandQueryRequest() 
{
}

CString MainWindowCommandQueryRequest::GetCommandParameter()
{
	return mKeyword; 
}

void MainWindowCommandQueryRequest::NotifyQueryComplete(
	bool isCancelled,
	std::vector<launcherapp::core::Command*>* result
)
{
	PostMessage(mHwnd, mMsg, isCancelled ? 1 : 0, (LPARAM)result);
}

uint32_t MainWindowCommandQueryRequest::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t MainWindowCommandQueryRequest::Release() 
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

