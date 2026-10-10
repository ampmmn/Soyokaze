#pragma once

#include "commands/common/AdhocCommandProviderBase.h"

#include <memory>
#include <vector>

namespace launcherapp {
namespace commands {
namespace office_favorites {

/**
 * @brief Word/Excel/PowerPoint のお気に入り(ピン止め)を提供するProvider
 * @note 設定の読み込み、レジストリ/JSONの変更監視、コマンドの生成を担当する
 */
class OfficeFavoritesProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
public:
	OfficeFavoritesProvider();
	virtual ~OfficeFavoritesProvider();

public:
	CString GetName() override;

	// 一時的なコマンドの準備を行うための初期化
	void PrepareAdhocCommands() override;
	// 一時的なコマンドを必要に応じて提供する
	virtual void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands);

	// Providerが扱うコマンド種別(表示名)を列挙
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(OfficeFavoritesProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace office_favorites
} // end of namespace commands
} // end of namespace launcherapp
