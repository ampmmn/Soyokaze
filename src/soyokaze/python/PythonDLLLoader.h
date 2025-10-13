#pragma once

#include <memory>

#include "python_proxy/PythonProxyIF.h"

class PythonDLLLoader
{
	PythonDLLLoader();
	~PythonDLLLoader();

public:
	static PythonDLLLoader* Get();
	bool Initialize();
	bool Finalize();
	bool IsAvailable();

	PythonProxyIF* GetLibrary();
	CString GetPythonExePath();
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

