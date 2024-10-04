#include "pch.h"
#include "ExtraCandidateProvider.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "commands/core/IFIDDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;
using ExtraCandidateSource = launcherapp::commands::core::ExtraCandidateSource;

namespace launcherapp {
namespace commands {
namespace extracandidate {


struct ExtraCandidateProvider::PImpl : public AppPreferenceListenerIF, public CommandRepositoryListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		ClearCommands();
	}

	void ClearCommands() {
		for (auto source : mSources) {
			source->Release();
		}
		mSources.clear();
	}

// AppPreferenceListenerIF
	void OnAppFirstBoot() override
	{
		OnAppNormalBoot();
	}
	void OnAppNormalBoot() override
	{
		auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
		cmdRepo->RegisterListener(this);
	}

	void OnAppPreferenceUpdated() override
	{
	}
	void OnAppExit() override
	{
		auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
		cmdRepo->UnregisterListener(this);
	}


// CommandRepositoryListenerIF
	void OnBeforeLoad() override
	{
		ClearCommands();
	}
	void OnNewCommand(launcherapp::core::Command* cmd) override
	{
		ExtraCandidateSource* newSource = nullptr;
		if (cmd->QueryInterface(IFID_EXTRACANDIDATESOURCE, (void**)&newSource) == false) {
			return;
		}
		ASSERT(newSource);
		mSources.push_back(newSource);
	}

	void OnDeleteCommand(Command* command) override
	{
		ExtraCandidateSource* newSource = nullptr;
		if (command->QueryInterface(IFID_EXTRACANDIDATESOURCE, (void**)&newSource) == false) {
			return;
		}

		auto it = std::find(mSources.begin(), mSources.end(), newSource);
		if (it != mSources.end()) {
			mSources.erase(it);
			command->Release();
		}
	}
	void OnLancuherActivate() override
	{
		for (auto source : mSources) {
			source->ClearCache();
		}
	}
	void OnLancuherUnactivate() override
	{
	}

	std::vector<ExtraCandidateSource*> mSources;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(ExtraCandidateProvider)

ExtraCandidateProvider::ExtraCandidateProvider() : in(std::make_unique<PImpl>())
{
}

ExtraCandidateProvider::~ExtraCandidateProvider()
{
}

CString ExtraCandidateProvider::GetName()
{
	return _T("");
}

// 一時的なコマンドを必要に応じて提供する
void ExtraCandidateProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	for (auto& source : in->mSources) {
		source->QueryCandidates(pattern, commands);
	}
}


} // end of namespace extracandidate
} // end of namespace commands
} // end of namespace launcherapp

