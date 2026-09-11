#include "pch.h"
#include "ShellUriCommandProvider.h"
#include "commands/shelluri/ShellUriCommand.h"
#include "commands/core/CommandRepository.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace shelluri {

struct ShellUriCommandProvider::PImpl
{
	ShellUriCommand* mCommandPtr{nullptr};
};

REGISTER_COMMANDPROVIDER(ShellUriCommandProvider)

/**
  Shell URIコマンドプロバイダを初期化する
*/
ShellUriCommandProvider::ShellUriCommandProvider() : in(std::make_unique<PImpl>())
{
	in->mCommandPtr = new ShellUriCommand();
}

/**
  Shell URIコマンドプロバイダを破棄する
*/
ShellUriCommandProvider::~ShellUriCommandProvider()
{
	if (in->mCommandPtr) {
		in->mCommandPtr->Release();
	}
}

/**
  Shell URIコマンドプロバイダの名前を取得する
  @return プロバイダ名
*/
CString ShellUriCommandProvider::GetName()
{
	return _T("ShellUriCommand");
}

/**
  Shell URIに一致するコマンドを候補一覧へ追加する
  @param[in] pattern 入力パターン
  @param[out] commands コマンド候補一覧
*/
void ShellUriCommandProvider::QueryAdhocCommands(
	Pattern* pattern,
	CommandQueryItemList& commands
)
{
	int level = in->mCommandPtr->Match(pattern);
	if (level != Pattern::Mismatch) {
		// 候補一覧がコマンドを保持できるよう参照カウントを増やす。
		in->mCommandPtr->AddRef();
		commands.Add(CommandQueryItem(level, in->mCommandPtr));
	}
}

/**
  Shell URIコマンドの表示名を列挙する
  @param[out] displayNames コマンド表示名の一覧
  @return 追加した表示名の数
*/
uint32_t ShellUriCommandProvider::EnumCommandDisplayNames(std::vector<CString>& displayNames)
{
	displayNames.push_back(ShellUriCommand::TypeDisplayName());
	return 1;
}

} // shelluri
} // commands
} // launcherapp
