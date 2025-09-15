#pragma once

#include <memory>
#include "actions/core/ActionParameterIF.h"

namespace launcherapp {
namespace commands {
namespace common {

class SubProcess
{
	using Parameter = launcherapp::actions::core::Parameter;

public:
	class Instance;
	using ProcessPtr = std::unique_ptr<Instance>;

public:
	SubProcess(Parameter* param);
	~SubProcess();

	void SetShowType(int showType);
	void SetRunAsAdmin();
	void SetWorkDirectory(const CString& dir);
	bool SetAdditionalEnvironment(const CString& name, const CString& value);

	bool Run(const CString& path, ProcessPtr& process);
	bool Run(const CString& path, const CString& paramStr, ProcessPtr& process);

	static bool IsRunningAsAdmin();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

class SubProcess::Instance
{
public:
	Instance(HANDLE hProcess);
	~Instance();

public:
	bool Wait(DWORD timeout);
	void SetErrorMessage(const CString& msg);
	CString GetErrorMessage();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}
}
