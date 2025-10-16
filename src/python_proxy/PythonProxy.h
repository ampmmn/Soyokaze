#pragma once

#include "python_proxy/PythonProxyIF.h"
#include <memory>

class PythonProxy : public PythonProxyIF
{
public:
	PythonProxy();
	virtual ~PythonProxy();

	void Finalize();

	virtual bool CompileTest(const char* src, char** result) override;
	bool Evaluate(const char* src, char** errMsg) override;
	bool EvalForCalculate(const char* src, char** result) override;
	void ReleaseBuffer(char* result) override;
	bool IsPyCmdAvailable() override;
	bool IsBusy() override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

