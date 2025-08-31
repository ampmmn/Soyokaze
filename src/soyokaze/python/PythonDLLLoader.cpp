#include "pch.h"
#include "PythonDLLLoader.h"
#include "python/PythonDLL.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"

static PythonDLLLoader* s_initialized = PythonDLLLoader::Get();


struct PythonDLLLoader::PImpl : public AppPreferenceListenerIF
{
	void OnAppFirstBoot() override
	{
	}

	void OnAppNormalBoot() override
	{
		mThisPtr->Initialize();
	}

	void OnAppPreferenceUpdated() override
	{
		mThisPtr->Initialize();
	}

	void OnAppExit() override
	{
		mThisPtr->Finalize();

		auto pref = AppPreference::Get();
		pref->UnregisterListener(this);
	}

	PythonDLLLoader* mThisPtr{nullptr};
	std::unique_ptr<PythonDLL> mDll;

	// 利用可能か?
	bool mIsAvailable{false};
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



PythonDLLLoader::PythonDLLLoader() : in(new PImpl)
{
	in->mThisPtr = this;

	auto pref = AppPreference::Get();
	pref->RegisterListener(in.get());
}

PythonDLLLoader::~PythonDLLLoader()
{
}

PythonDLLLoader* PythonDLLLoader::Get()
{
	static PythonDLLLoader inst;
	return &inst;
}

bool PythonDLLLoader::Initialize()
{
	auto pref = AppPreference::Get();
	auto dllPath = pref->GetPythonDLLPath();
	if (dllPath.IsEmpty()) {
		in->mDll.reset();
		in->mIsAvailable = false;
		return false;
	}

	in->mDll.reset(new PythonDLL());
	if (in->mDll->LoadDLL(dllPath) == false) {
		in->mIsAvailable = false;
		return false;
	}

	in->mIsAvailable = true;
	return true;
}

bool PythonDLLLoader::Finalize()
{
	in->mDll.reset();
	return true;
}

bool PythonDLLLoader::IsAvailable()
{
	return in->mIsAvailable;
}

PythonDLL* PythonDLLLoader::GetLibrary()
{
	return in->mDll.get();
}
