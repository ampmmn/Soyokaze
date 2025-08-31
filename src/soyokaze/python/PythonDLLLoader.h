#pragma once

#include <memory>

#include "python/PythonDLL.h"

class PythonDLLLoader
{
	PythonDLLLoader();
	~PythonDLLLoader();

public:
	static PythonDLLLoader* Get();
	bool Initialize();
	bool Finalize();
	bool IsAvailable();

	PythonDLL* GetLibrary();
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

