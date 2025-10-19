#include "pch.h"
#include "ScintillaDLLLoader.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/Path.h"

using ILexer = Scintilla::ILexer5;

namespace launcherapp { namespace commands { namespace py_extension {

struct ScintillaDLLLoader::PImpl : public AppPreferenceListenerIF
{
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override {}

	void OnAppExit() override
	{
		mThisPtr->Finalize();

		auto pref = AppPreference::Get();
		pref->UnregisterListener(this);
	}

	ScintillaDLLLoader* mThisPtr{nullptr};
	HMODULE mScintillaDLL{nullptr};
	HMODULE mLexillaDLL{nullptr};
};


ScintillaDLLLoader::ScintillaDLLLoader() : in(new PImpl())
{
	in->mThisPtr = this;

	auto pref = AppPreference::Get();
	pref->RegisterListener(in.get());
}

ScintillaDLLLoader::~ScintillaDLLLoader()
{
}

ScintillaDLLLoader* ScintillaDLLLoader::GetInstance()
{
	static ScintillaDLLLoader inst;
	return &inst;
}

bool ScintillaDLLLoader::Initialize()
{
	Path lexillaDllPath(Path::MODULEFILEDIR, _T("Lexilla.dll"));
	in->mLexillaDLL = LoadLibrary((LPCTSTR)lexillaDllPath);
	Path scintillaDllPath(Path::MODULEFILEDIR, _T("Scintilla.dll"));
	in->mScintillaDLL = LoadLibrary((LPCTSTR)scintillaDllPath);

	bool isOK = in->mScintillaDLL != nullptr && in->mLexillaDLL != nullptr;

	spdlog::info("scintilla.dll / lexilla.dll load : {}", isOK);

	return isOK;
}

bool ScintillaDLLLoader::Finalize()
{
	if (in->mScintillaDLL) {
		FreeLibrary(in->mScintillaDLL);
		in->mScintillaDLL = nullptr;
	}
	if (in->mLexillaDLL) {
		FreeLibrary(in->mLexillaDLL);
		in->mLexillaDLL = nullptr;
	}
	spdlog::info("scintilla.dll / lexilla.dll are unloaded.");
	return true;
}

ILexer* ScintillaDLLLoader::CreateLexer(const char* langName)
{
	if (in->mLexillaDLL == nullptr) {
		return nullptr;
	}

	using LPCREATELEXER = ILexer* (*)(const char*);
	auto CreateLexerReal = (LPCREATELEXER)GetProcAddress(in->mLexillaDLL, "CreateLexer");
	if (CreateLexerReal == nullptr) {
		return nullptr;
	}
	return CreateLexerReal(langName);
}


}}}
