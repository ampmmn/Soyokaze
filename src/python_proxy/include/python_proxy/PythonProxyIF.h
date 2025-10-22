#pragma once

struct PythonProxyIF
{
	virtual bool CompileTest(const char* src, char** errMsg) = 0;
	virtual bool Evaluate(const char* src, const char** argv, char** errMsg) = 0;
	virtual bool EvalForCalculate(const char* src, char** result) = 0;
	virtual void ReleaseBuffer(char* result) = 0;
	virtual bool IsPyCmdAvailable() = 0;
	virtual bool IsBusy() = 0;
};


