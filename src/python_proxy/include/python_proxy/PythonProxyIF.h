#pragma once

struct PythonProxyIF
{
	virtual bool Evaluate(const wchar_t* src, char** result) = 0;
	virtual void ReleaseBuffer(char* result) = 0;
};


