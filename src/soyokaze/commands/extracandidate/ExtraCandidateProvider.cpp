#include "pch.h"
#include "ExtraCandidateProvider.h"
#include "commands/core/ExtraCandidateSourceIF.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandRepositoryListenerIF.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "core/IFIDDefine.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = launcherapp::core::CommandRepository;
using CommandRepositoryListenerIF = launcherapp::core::CommandRepositoryListenerIF;
using ExtraCandidateSource = launcherapp::commands::core::ExtraCandidateSource;

namespace launcherapp { namespace commands { namespace extracandidate {


struct ExtraCandidateProvider::PImpl :
 	public AppPreferenceListenerIF,
 	public CommandRepositoryListenerIF,
	public LauncherWindowEventListenerIF

{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
		LauncherWindowEventDispatcher::Get()->AddListener(this);
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
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);

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
		RefPtr<ExtraCandidateSource> newSource;
		if (cmd->QueryInterface(IFID_EXTRACANDIDATESOURCE, (void**)&newSource) == false) {
			return;
		}
		ASSERT(newSource.get());
		mSources.push_back(newSource.release());
	}

	void OnDeleteCommand(Command* command) override
	{
		RefPtr<ExtraCandidateSource> newSource;
		if (command->QueryInterface(IFID_EXTRACANDIDATESOURCE, (void**)&newSource) == false) {
			return;
		}

		auto it = std::find(mSources.begin(), mSources.end(), newSource.get());
		if (it != mSources.end()) {
			mSources.erase(it);
			// mSourcesで抱えていたぶんの参照カウントを減らす
			command->Release();
		}
	}
	void OnPatternReloaded() override {}

// LauncherWindowEventListenerIF
	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {}

	void OnLauncherActivate() override
	{
		// 入力欄が表示されるタイミングでキャッシュをリセット
		for (auto source : mSources) {
			source->ClearCache();
		}
	}
	void OnLauncherUnactivate() override
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
	return _T("ExtraCandidate");
}

// 一時的なコマンドを必要に応じて提供する
void ExtraCandidateProvider::QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands)
{
	CString name;
	for (auto& source : in->mSources) {

		RefPtr<Command> cmd;
		if (source->QueryInterface(IFID_COMMAND, (void**)&cmd)) {
			name = cmd->GetName();
		}
		else {
			name = _T("Unknown");
		}

		PERFLOG(_T("ExtracCandidate Start name:{}"), (LPCTSTR)name);
		spdlog::stopwatch sw;

		source->QueryCandidates(pattern, commands);

		PERFLOG("ExtracCandidate End {0:.6f} s.", sw);
	}
}


}}} // end of namespace launcherapp::commands::extracandidate

