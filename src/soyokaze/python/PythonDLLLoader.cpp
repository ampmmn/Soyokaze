#include "pch.h"
#include "PythonDLLLoader.h"
#include "python/PythonDLL.h"
#include "setting/AppPreference.h"

struct PythonDLLLoader::PImpl
{
	std::unique_ptr<PythonDLL> mDll;
};

PythonDLLLoader::PythonDLLLoader() : in(new PImpl)
{
}

PythonDLLLoader::~PythonDLLLoader()
{
}

PythonDLLLoader* PythonDLLLoader::Get()
{
	static PythonDLLLoader inst;
	return &inst;
}

bool PythonDLLLoader::Load()
{
	auto pref = AppPreference::Get();
	auto dllPath = pref->GetPythonDLLPath();
	if (dllPath.IsEmpty()) {
		in->mDll.reset();
		return false;
	}

	in->mDll.reset(new PythonDLL());
	return in->mDll->SetDLLPath(dllPath);
}


