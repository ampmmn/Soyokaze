#pragma once

#include <memory>

class PythonDLL
{
public:
	PythonDLL();
	~PythonDLL();

	bool SetDLLPath(const CString& dllPath);
	bool Evaluate(const CString& src, CString& result);
	bool SingleInput(const CString& src, CString& result);
	bool CanCompile(const CString& src);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

