#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/ContextMenuSourceIF.h"
#include "OfficeFavorites.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace office_favorites {

/**
 * @brief Office のお気に入り1件に対応するコマンド
 * @note Enter: ファイルを開く / Ctrl+Enter: パスを開く(URLの場合はURLを開く) / Alt+Enter: 無し
 */
class OfficeFavoritesCommand :
	public launcherapp::commands::common::AdhocCommandBase,
	public launcherapp::commands::core::ContextMenuSource
{
public:
	OfficeFavoritesCommand(const OfficeFavoriteItem& item);
	virtual ~OfficeFavoritesCommand();

	CString GetTypeDisplayName() override;
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	HICON GetIcon() override;
	launcherapp::core::Command* Clone() override;

// ContextMenuSource
	// メニューの項目数を取得する
	int GetMenuItemCount() override;
	// メニューに対応するアクションを取得する
	bool GetMenuItem(int index, Action** action) override;

// UnknownIF
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(OfficeFavoritesCommand)

public:
	// 表示名("お気に入り(アプリ名)")を取得する
	static CString TypeDisplayName(const CString& appName);

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace office_favorites
} // end of namespace commands
} // end of namespace launcherapp
