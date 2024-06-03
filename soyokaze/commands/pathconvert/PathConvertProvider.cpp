#include "pch.h"
#include "PathConvertProvider.h"
#include "commands/pathconvert/AppSettingPathConvertPage.h"
#include "commands/pathconvert/GitBashToLocalPathAdhocCommand.h"
#include "commands/pathconvert/LocalToGitBashPathAdhocCommand.h"
#include "commands/pathconvert/FileProtocolConvertAdhocCommand.h"
#include "commands/pathconvert/P4PathConvertAdhocCommand.h"
#include "commands/pathconvert/P4AppSettings.h"
#include "commands/core/CommandRepository.h"
#include "commands/core/CommandParameter.h"
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
		auto pref = AppPreference::Get();
		mIsEnableGitBash = pref->IsEnableGitBashPath();
	}
	void OnAppExit() override {}

	GitBashToLocalPathAdhocCommand* mGitBashToLocalPathCmdPtr;
	LocalToGitBashPathAdhocCommand* mLocalToGitBashPathCmdPtr;
	FileProtocolConvertAdhocCommand* mFileProtocolCmdPtr;
	std::vector<P4PathConvertAdhocCommand*> mP4PathCommands;
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
	in->mFileProtocolCmdPtr = new FileProtocolConvertAdhocCommand();
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
	if (in->mFileProtocolCmdPtr) {
		in->mFileProtocolCmdPtr->Release();
	}
	for (auto& p4cmd : in->mP4PathCommands) {
		p4cmd->Release();
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

		// p4設定の読み込み
		// ToDo: ON/OFFを切り替えられるようにする
		P4AppSettings p4AppSettings;
		std::vector<P4AppSettings::ITEM> items;
		p4AppSettings.GetItems(items);
		for (auto& item : items) {
			in->mP4PathCommands.push_back(new P4PathConvertAdhocCommand(item));
		}
	}

	int level = in->mFileProtocolCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mFileProtocolCmdPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mFileProtocolCmdPtr));
		return;
	}

	if (in->mIsEnableGitBash == false) {
		return;
	}

	// git-bashのパス表記をローカルパス表記を変換するコマンド
	level = in->mGitBashToLocalPathCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mGitBashToLocalPathCmdPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mGitBashToLocalPathCmdPtr));
		return;
	}

	// ローカルパス表記をgit-bashのパス表記に変換するコマンド
	level = in->mLocalToGitBashPathCmdPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mLocalToGitBashPathCmdPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mLocalToGitBashPathCmdPtr));
	}

	for (auto& p4cmd : in->mP4PathCommands) {
		level = p4cmd->Match(pattern);
		if (level != Pattern::Mismatch) {
			p4cmd->AddRef();
			commands.push_back(CommandQueryItem(level, p4cmd));
			return;
		}
	}
}

/**
 	設定ページを取得する
 	@return true 成功  false失敗
 	@param[in]  parent 親ウインドウ
 	@param[out] pages  設定ページリスト
*/
bool PathConvertProvider::CreateSettingPages(
	CWnd* parent,
	std::vector<SettingPage*>& pages
)
{
	// 必要に応じて実装する
	pages.push_back(new AppSettingPathConvertPage(parent));
	return true;
}


} // end of namespace pathconvert
} // end of namespace commands
} // end of namespace launcherapp

