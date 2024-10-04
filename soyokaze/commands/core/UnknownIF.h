#pragma once

#include "commands/core/IFID.h"

namespace launcherapp {
namespace core {

class UnknownIF
{
public:
	virtual bool QueryInterface(const IFID& ifid, void** cmd) = 0;

	virtual uint32_t AddRef() = 0;
	virtual uint32_t Release() = 0;
};


} // end of namespace core
} // end of namespace launcherapp

