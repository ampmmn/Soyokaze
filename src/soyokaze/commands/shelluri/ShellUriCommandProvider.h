#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace shelluri {

class ShellUriCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	ShellUriCommandProvider();
	virtual ~ShellUriCommandProvider();

public:
	/**
	  Shell URIコマンドプロバイダの名前を取得する
	  @return プロバイダ名
	*/
	CString GetName() override;

	/**
	  Shell URIコマンドを検索して候補に追加する
	  @param[in] pattern 入力パターン
	  @param[out] commands コマンド候補一覧
	*/
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands) override;

	/**
	  Shell URIコマンドの表示名を列挙する
	  @param[out] displayNames コマンド表示名の一覧
	  @return 追加した表示名の数
	*/
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(ShellUriCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace shelluri
} // end of namespace commands
} // end of namespace launcherapp
