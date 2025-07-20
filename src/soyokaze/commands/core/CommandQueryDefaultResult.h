#pragma once

#include "commands/core/CommandQueryRequest.h"
#include "commands/core/CommandQueryItem.h"
#include <vector>

namespace launcherapp { namespace commands { namespace core {

class CommandQueryDefaultResult : public CommandQueryResult
{
	CommandQueryDefaultResult();
	~CommandQueryDefaultResult();
public:
	static CommandQueryDefaultResult* Create();
	void Add(const launcherapp::CommandQueryItem& item);

	size_t GetCount() override;
	bool IsEmpty() override;
	bool Get(size_t index, launcherapp::core::Command** cmd, int* matchLevel) override;
	launcherapp::core::Command* GetItem(size_t index, int* matchLevel = nullptr) override;
	uint32_t AddRef() override;
	uint32_t Release() override;

private:
	std::vector<launcherapp::CommandQueryItem> mResultItems;
	uint32_t mRefCount{1};
};


}}}

