#pragma once

#include <memory>

class PythonDLLLoader
{
	PythonDLLLoader();
	~PythonDLLLoader();

public:
	static PythonDLLLoader* Get();
	bool Load();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

