#pragma once

namespace soyokaze {
namespace core {
	class CommandParameter;
}
}

namespace soyokaze {
namespace commands {
namespace common {

class SubProcess
{
	using CommandParameter = soyokaze::core::CommandParameter;

public:
	class Instance;
	using ProcessPtr = std::unique_ptr<Instance>;

public:
	SubProcess(const CommandParameter& param);
	~SubProcess();

	void SetShowType(int showType);
	void SetRunAsAdmin();
	void SetWorkDirectory(const CString& dir);

	bool Run(const CString& path, ProcessPtr& process);
	bool Run(const CString& path, const CString& paramStr, ProcessPtr& process);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

class SubProcess::Instance
{
public:
	Instance(SHELLEXECUTEINFO si);
	~Instance();

public:
	bool Wait(DWORD timeout);
	void SetErrorMessage(const CString& msg);
	CString GetErrorMessage();

private:
	SHELLEXECUTEINFO mShellExecuteInfo;
	CString mErrMsg;
};

}
}
}
