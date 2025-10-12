#pragma once

#include "python_proxy/PythonProxyIF.h"
#include <memory>

class PythonProxy : public PythonProxyIF
{
public:
	PythonProxy();
	virtual ~PythonProxy();

	bool Evaluate(const wchar_t* src, char** result) override;
	void ReleaseBuffer(char* result) override;

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

