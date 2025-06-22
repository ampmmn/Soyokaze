#pragma once

#include "commands/core/CommandQueryRequest.h"
#include <mutex>

namespace launcherapp { namespace commands { namespace history {

class CommandQueryRequest : public launcherapp::commands::core::CommandQueryRequest
{
public:
	CommandQueryRequest(const CString& keyword);
	~CommandQueryRequest();

	bool WaitComplete(DWORD timeout);
	bool GetResult(std::vector<launcherapp::core::Command*>& result);

	CString GetCommandParameter() override;
	void NotifyQueryComplete(bool isCancelled, std::vector<launcherapp::core::Command*>* result) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	uint32_t mRefCount{1};
	CString mKeyword;
	HANDLE mEventHandle{nullptr};
	std::vector<launcherapp::core::Command*>* mResult{nullptr};
	std::mutex mMutex;
};

}}} // end of namespace launcherapp::commands::history
