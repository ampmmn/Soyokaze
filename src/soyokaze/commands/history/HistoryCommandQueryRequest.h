#pragma once

#include "commands/core/CommandQueryRequest.h"
#include <mutex>

namespace launcherapp { namespace commands { namespace history {

class CommandQueryRequest : public launcherapp::commands::core::CommandQueryRequest
{
	using CommandQueryResult = launcherapp::commands::core::CommandQueryResult;
public:
	CommandQueryRequest(const CString& keyword);
	~CommandQueryRequest();

	bool WaitComplete(DWORD timeout);
	bool GetResult(CommandQueryResult** result);

	CString GetCommandParameter() override;
	void NotifyQueryComplete(bool isCancelled, CommandQueryResult* result) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	uint32_t mRefCount{1};
	CString mKeyword;
	HANDLE mEventHandle{nullptr};
	CommandQueryResult* mResult{nullptr};
	std::mutex mMutex;
};

}}} // end of namespace launcherapp::commands::history
