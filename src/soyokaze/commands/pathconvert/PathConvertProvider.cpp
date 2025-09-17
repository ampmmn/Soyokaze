#include "pch.h"
#include "PathConvertProvider.h"
#include "commands/pathconvert/AppSettingPathConvertPage.h"
#include "commands/pathconvert/GitBashToLocalPathAdhocCommand.h"
#include "commands/pathconvert/LocalToGitBashPathAdhocCommand.h"
#include "commands/pathconvert/FileProtocolConvertAdhocCommand.h"
#include "commands/core/CommandRepository.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace pathconvert {


using CommandRepository = launcherapp::core::CommandRepository;

struct PathConvertProvider::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		Load();
	}
	void OnAppExit() override {}

	void Load()
	{
		auto pref = AppPreference::Get();
		mIsEnableGitBash = pref->IsEnableGitBashPath();
	}

	GitBashToLocalPathAdhocCommand* mGitBashToLocalPathCmdPtr{nullptr};
	LocalToGitBashPathAdhocCommand* mLocalToGitBashPathCmdPtr{nullptr};
	FileProtocolConvertAdhocCommand* mFileProtocolCmdPtr{nullptr};

	bool mIsEnableGitBash{false};

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PathConvertProvider)


PathConvertProvider::PathConvertProvider() : in(std::make_unique<PImpl>())
{
	in->mGitBashToLocalPathCmdPtr = new GitBashToLocalPathAdhocCommand();
	in->mLocalToGitBashPathCmdPtr = new LocalToGitBashPathAdhocCommand();
	in->mFileProtocolCmdPtr = new FileProtocolConvertAdhocCommand();
}

PathConvertProvider::~PathConvertProvider()
{
	if (in->mGitBashToLocalPathCmdPtr) {
		in->mGitBashToLocalPathCmdPtr->Release();
	}
	if (in->mLocalToGitBashPathCmdPtr) {
		in->mLocalToGitBashPathCmdPtr->Release();
	}
	if (in->mFileProtocolCmdPtr) {
		in->mFileProtocolCmdPtr->Release();
	}
}

// コマンドの読み込み
void PathConvertProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	UNREFERENCED_PARAMETER(cmdFile);

}

CString PathConvertProvider::GetName()
{
	return _T("PathConvert");
}

// 一時的なコマンドの準備を行うための初期化
void PathConvertProvider::PrepareAdhocCommands()
{
	// 初回呼び出し時に設定よみこみ
	in->Load();
}

// 一時的なコマンドを必要に応じて提供する
void PathConvertProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	int level = in->mFileProtocolCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mFileProtocolCmdPtr->AddRef();
		commands.Add(CommandQueryItem(level, in->mFileProtocolCmdPtr));
		return;
	}

	if (in->mIsEnableGitBash == false) {
		return;
	}

	// git-bashのパス表記をローカルパス表記を変換するコマンド
	level = in->mGitBashToLocalPathCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mGitBashToLocalPathCmdPtr->AddRef();
		commands.Add(CommandQueryItem(level, in->mGitBashToLocalPathCmdPtr));
		return;
	}

	// ローカルパス表記をgit-bashのパス表記に変換するコマンド
	level = in->mLocalToGitBashPathCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mLocalToGitBashPathCmdPtr->AddRef();
		commands.Add(CommandQueryItem(level, in->mLocalToGitBashPathCmdPtr));
	}
}

// Providerが扱うコマンド種別(表示名)を列挙
uint32_t PathConvertProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(FileProtocolConvertAdhocCommand::TypeDisplayName());
	displayNames.push_back(GitBashToLocalPathAdhocCommand::TypeDisplayName());
	displayNames.push_back(LocalToGitBashPathAdhocCommand::TypeDisplayName());
	return 3;
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp

