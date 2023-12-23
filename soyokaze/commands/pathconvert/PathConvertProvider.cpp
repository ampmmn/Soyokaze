#include "pch.h"
#include "PathConvertProvider.h"
#include "commands/pathconvert/GitBashToLocalPathAdhocCommand.h"
#include "commands/pathconvert/LocalToGitBashPathAdhocCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreferenceListenerIF.h"
#include "AppPreference.h"
#include "resource.h"
#include <list>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace pathconvert {


using CommandRepository = soyokaze::core::CommandRepository;

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
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsEnableGitBash = pref->IsEnableGitBashPath();
	}
	void OnAppExit() override {}

	GitBashToLocalPathAdhocCommand* mGitBashToLocalPathCmdPtr;
	LocalToGitBashPathAdhocCommand* mLocalToGitBashPathCmdPtr;
	// 初回呼び出しフラグ(初回呼び出し時に設定をロードするため)
	bool mIsFirstCall;

	bool mIsEnableGitBash;

};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PathConvertProvider)


PathConvertProvider::PathConvertProvider() : in(std::make_unique<PImpl>())
{
	in->mGitBashToLocalPathCmdPtr = new GitBashToLocalPathAdhocCommand();
	in->mLocalToGitBashPathCmdPtr = new LocalToGitBashPathAdhocCommand();
	in->mIsFirstCall = true;
	in->mIsEnableGitBash = false;
}

PathConvertProvider::~PathConvertProvider()
{
	if (in->mGitBashToLocalPathCmdPtr) {
		in->mGitBashToLocalPathCmdPtr->Release();
	}
	if (in->mLocalToGitBashPathCmdPtr) {
		in->mLocalToGitBashPathCmdPtr->Release();
	}
}

// コマンドの読み込み
void PathConvertProvider::LoadCommands(
	CommandFile* cmdFile
)
{
}

CString PathConvertProvider::GetName()
{
	return _T("PathConvert");
}

// 一時的なコマンドを必要に応じて提供する
void PathConvertProvider::QueryAdhocCommands(
	Pattern* pattern,
 	CommandQueryItemList& commands
)
{
	if (in->mIsFirstCall) {
		// 初回呼び出し時に設定よみこみ
		auto pref = AppPreference::Get();
		in->mIsFirstCall = false;
		in->mIsEnableGitBash = pref->IsEnableGitBashPath();
	}

	if (in->mIsEnableGitBash == false) {
		return;
	}

	// git-bashのパス表記をローカルパス表記を変換するコマンド
	int level = in->mGitBashToLocalPathCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mGitBashToLocalPathCmdPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mGitBashToLocalPathCmdPtr));
	}
	// ローカルパス表記をgit-bashのパス表記に変換するコマンド
	level = in->mLocalToGitBashPathCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mLocalToGitBashPathCmdPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mLocalToGitBashPathCmdPtr));
	}
}

} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace soyokaze

