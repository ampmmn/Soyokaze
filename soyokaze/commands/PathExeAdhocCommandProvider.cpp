#include "pch.h"
#include "PathExeAdhocCommandProvider.h"
#include "commands/ExecutableFileCommand.h"
#include "core/CommandRepository.h"
#include "core/CommandParameter.h"
#include "AppPreference.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using CommandRepository = soyokaze::core::CommandRepository;


struct PathExeAdhocCommandProvider::PImpl
{
	uint32_t mRefCount;
	// 環境変数PATHにあるexeを実行するためのコマンド
	ExecutableFileCommand* mExeCommandPtr;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

REGISTER_COMMANDPROVIDER(PathExeAdhocCommandProvider)


PathExeAdhocCommandProvider::PathExeAdhocCommandProvider() : in(new PImpl)
{
	in->mRefCount = 1;
	in->mExeCommandPtr = new ExecutableFileCommand();
}

PathExeAdhocCommandProvider::~PathExeAdhocCommandProvider()
{
	if (in->mExeCommandPtr) {
		in->mExeCommandPtr->Release();
	}
}

// 初回起動の初期化を行う
void PathExeAdhocCommandProvider::OnFirstBoot()
{
	// 何もしない
}


// コマンドの読み込み
void PathExeAdhocCommandProvider::LoadCommands(
	CommandFile* cmdFile
)
{
	// サポートしない
}

CString PathExeAdhocCommandProvider::GetName()
{
	return _T("PathExeAdhocCommand");
}

// 作成できるコマンドの種類を表す文字列を取得
CString PathExeAdhocCommandProvider::GetDisplayName()
{
	// サポートしない
	return _T("");
}

// コマンドの種類の説明を示す文字列を取得
CString PathExeAdhocCommandProvider::GetDescription()
{
	// サポートしない
	return _T("");
}

// コマンド新規作成ダイアログ
bool PathExeAdhocCommandProvider::NewDialog(const CommandParameter* param)
{
	// サポートしない
	return false;
}

// 非公開コマンドかどうか(新規作成対象にしない)
bool PathExeAdhocCommandProvider::IsPrivate() const
{
	return true;
}

// 一時的なコマンドを必要に応じて提供する
void PathExeAdhocCommandProvider::QueryAdhocCommands(Pattern* pattern, std::vector<CommandQueryItem>& commands)
{
	int level = in->mExeCommandPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		in->mExeCommandPtr->AddRef();
		commands.push_back(CommandQueryItem(level, in->mExeCommandPtr));
	}
}

// Provider間の優先順位を表す値を返す。小さいほど優先
uint32_t PathExeAdhocCommandProvider::PathExeAdhocCommandProvider::GetOrder() const
{
	return 1000;
}

uint32_t PathExeAdhocCommandProvider::PathExeAdhocCommandProvider::AddRef()
{
	return ++in->mRefCount;
}

uint32_t PathExeAdhocCommandProvider::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

