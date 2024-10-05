#pragma once

#include "commands/core/CommandIF.h"

#pragma warning( disable : 4250)

namespace launcherapp {
namespace commands {
namespace common {

class UserCommandBase : virtual public launcherapp::core::Command
{
public:
	UserCommandBase();
	virtual ~UserCommandBase();

	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	CString GetErrorString() override;

	bool IsEditable() override;
	bool IsDeletable() override;

	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	static bool GetNamedParamString(Parameter* param, LPCTSTR name, CString& value);

protected:
	uint32_t mRefCount = 1;
};

}
}
}


