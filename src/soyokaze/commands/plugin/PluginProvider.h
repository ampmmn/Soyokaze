#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"
#include <memory>
#include "PluginModule.h"

namespace launcherapp {
namespace commands {
namespace plugin {

class PluginProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	/**
	  プラグインコマンドプロバイダを生成する
	*/
	PluginProvider();
	/**
	  プラグインコマンドプロバイダを破棄する
	*/
	~PluginProvider() override;

public:
	static PluginProvider* GetInstance();
	/**
	  プラグインコマンドプロバイダの名前を取得する
	  @return プロバイダ名
	*/
	CString GetName() override;
	/**
	  プラグインDLLを読み込み、コマンド検索の準備を行う
	*/
	void PrepareAdhocCommands() override;
	/**
	  入力パターンに一致するプラグインコマンドを検索する
	  @param[in] pattern 検索パターン
	  @param[out] commands 検索結果を追加するリスト
	*/
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands) override;
	/**
	  保存対象のコマンド表示名を列挙する
	  @param[out] displayNames コマンド表示名の格納先
	  @return 列挙した表示名の数
	*/
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;
	void EnumPlugins(std::vector<PluginModulePtr>& plugins);

	DECLARE_COMMANDPROVIDER(PluginProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // namespace plugin
} // namespace commands
} // namespace launcherapp
