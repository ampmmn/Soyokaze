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

	// パス・パラメータが既に展開済みかどうかを設定する
	// (展開済みの場合は、実行時に再度展開しない)
	void SetExpanded(bool expanded);

	// マクロを展開するかどうかを設定する
	void SetExpandMacros(bool expandMacros);

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
