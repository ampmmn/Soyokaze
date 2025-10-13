#include "pch.h"
#include "PythonDLLLoader.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/Path.h"

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

	HMODULE mProxyDLL{nullptr};
	PythonDLLLoader* mThisPtr{nullptr};

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
	Finalize();
}

PythonDLLLoader* PythonDLLLoader::Get()
{
	static PythonDLLLoader inst;
	return &inst;
}

bool PythonDLLLoader::Initialize()
{
	if (in->mIsAvailable) {
		// 既に初期化済
		return true;
	}

	auto pref = AppPreference::Get();
	Path dllPath(pref->GetPythonDLLPath());
	if (dllPath.IsEmptyPath()|| dllPath.FileExists() == false) {
		in->mIsAvailable = false;
		spdlog::info("python_proxy dll path empty.");
		return false;
	}

	if (dllPath.IsDirectory() == false) {
		dllPath.RemoveFileSpec();
		if (dllPath.IsDirectory() == false) {
			in->mIsAvailable = false;
			spdlog::warn("python_proxy dll path does not exist.");
			return false;
		}
	}

	// 初期化
	auto cookie = AddDllDirectory((LPCTSTR)dllPath);
	Path proxy_path(Path::MODULEFILEDIR, _T("python_proxy.dll"));
	HMODULE h = LoadLibraryEx((LPCTSTR)proxy_path, nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS);
	BOOL isOK = RemoveDllDirectory(cookie);
	spdlog::debug("RemoveDllDirectory result: {}", isOK != FALSE);

	if (h == nullptr) {
		spdlog::error("Failed to load python_proxy.dll");
		return false;
	}

	in->mProxyDLL = h;
	in->mIsAvailable = true;
	return true;
}

bool PythonDLLLoader::Finalize()
{
	if (in->mProxyDLL) {
		FreeLibrary(in->mProxyDLL);
		in->mProxyDLL = nullptr;
	}
	return true;
}

bool PythonDLLLoader::IsAvailable()
{
	return in->mIsAvailable;
}

PythonProxyIF* PythonDLLLoader::GetLibrary()
{
	if (in->mProxyDLL == nullptr) {
		return nullptr;
	}

	using LPFUNCGETPROXY = int(*)(void**);
	auto pythonproxy_GetProxyObject = (LPFUNCGETPROXY)GetProcAddress(in->mProxyDLL, "pythonproxy_GetProxyObject");
	if (pythonproxy_GetProxyObject == nullptr) {
		return nullptr;
	}

	PythonProxyIF* proxy = nullptr;
	pythonproxy_GetProxyObject((void**)&proxy);

	return proxy;
}

CString PythonDLLLoader::GetPythonExePath()
{
	auto pref = AppPreference::Get();
	Path dllPath(pref->GetPythonDLLPath());
	if (dllPath.IsEmptyPath()|| dllPath.FileExists() == false) {
		return _T("");
	}

	dllPath.RemoveFileSpec();
	dllPath.Append(_T("python.exe"));
	return (LPCTSTR)dllPath;
}

