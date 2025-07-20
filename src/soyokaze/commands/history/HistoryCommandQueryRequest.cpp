#include "pch.h"
#include "HistoryCommandQueryRequest.h"

namespace launcherapp { namespace commands { namespace history {

CommandQueryRequest::CommandQueryRequest(
	const CString& keyword
) :
	mKeyword(keyword), mEventHandle(CreateEvent(nullptr, TRUE, FALSE, nullptr))
{
}

CommandQueryRequest::~CommandQueryRequest() 
{
	if (mEventHandle) {
		CloseHandle(mEventHandle);
		mEventHandle = nullptr;
	}
	if (mResult){
		mResult->Release();
		mResult = nullptr;
	}
}

bool CommandQueryRequest::WaitComplete(DWORD timeout)
{
	return WaitForSingleObject(mEventHandle, timeout) == WAIT_OBJECT_0;
}

bool CommandQueryRequest::GetResult(CommandQueryResult** result)
{
	std::lock_guard<std::mutex> lock(mMutex);

	if (mResult == nullptr) {
		return false;
	}

	mResult->AddRef();
	*result = mResult;

	return true;
}

CString CommandQueryRequest::GetCommandParameter()
{
	return mKeyword; 
}

void CommandQueryRequest::NotifyQueryComplete(
	bool isCancelled,
	CommandQueryResult* result
)
{
	UNREFERENCED_PARAMETER(isCancelled);

	mResult = result;
	mResult->AddRef();
	SetEvent(mEventHandle);
}

uint32_t CommandQueryRequest::AddRef()
{
	return (uint32_t)InterlockedIncrement(&mRefCount);
}

uint32_t CommandQueryRequest::Release() 
{
	auto n = InterlockedDecrement(&mRefCount);
	if (n == 0) {
		delete this;
	}
	return n;
}

}}} // end of namespace launcherapp::commands::history
