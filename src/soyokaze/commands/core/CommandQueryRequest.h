#pragma once

#include <vector>
#include "commands/core/CommandIF.h"

namespace launcherapp { namespace commands { namespace core {

class CommandQueryRequest
{
public:
	virtual ~CommandQueryRequest() {}

	virtual CString GetCommandParameter() = 0;
	virtual void NotifyQueryComplete(bool isCancelled, std::vector<launcherapp::core::Command*>* result) = 0;
	virtual uint32_t AddRef() = 0;
	virtual uint32_t Release() = 0;
};

}}}

