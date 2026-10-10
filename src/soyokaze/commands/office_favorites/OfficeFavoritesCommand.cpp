#include "pch.h"
#include "framework.h"
#include "OfficeFavoritesCommand.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using ShowPropertiesAction = launcherapp::actions::builtin::ShowPropertiesAction;

namespace launcherapp {
namespace commands {
namespace office_favorites {

struct OfficeFavoritesCommand::PImpl
{
	// お気に入りの情報
	OfficeFavoriteItem mItem;
};

IMPLEMENT_ADHOCCOMMAND_UNKNOWNIF(OfficeFavoritesCommand)

OfficeFavoritesCommand::OfficeFavoritesCommand(const OfficeFavoriteItem& item) :
	AdhocCommandBase(item.GetName(), item.IsURL() ? OfficeFavorites::DecodeUrl(item.mPath) : item.mPath),
	in(std::make_unique<PImpl>())
{
	in->mItem = item;
}

OfficeFavoritesCommand::~OfficeFavoritesCommand()
{
}

CString OfficeFavoritesCommand::GetTypeDisplayName()
{
	return TypeDisplayName(OfficeFavorites::AppTypeName(in->mItem.GetAppType()));
}

// Enter/Ctrl+Enter に対応するアクションを取得する
bool OfficeFavoritesCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		// Enter: ファイルを開く
		if (in->mItem.IsURL() == false) {
			// ローカルファイルはシェルの関連付けで開く
			*action = new ExecuteAction(in->mItem.mPath);
			return true;
		}

		// URLの場合は対応するOfficeアプリに引数としてURLを渡す
		CString exePath;
		if (OfficeFavorites::GetAppExePath(in->mItem.GetAppType(), exePath)) {
			*action = new ExecuteAction(exePath, in->mItem.mPath);
		}
		else {
			// 実行ファイルが見つからない場合は既定のアプリケーションに任せる
			*action = new ExecuteAction(in->mItem.mPath);
		}
		return true;
	}
	else if (modifierFlags == MOD_CONTROL) {
		if (in->mItem.IsURL() == false) {
			*action = new OpenPathInFilerAction(in->mItem.mPath);
			return true;
		}
	}
	else if (modifierFlags == MOD_ALT) {
		if (in->mItem.IsURL() == false) {
			*action = new ShowPropertiesAction(in->mItem.mPath);
			return true;
		}
	}
	return false;
}

HICON OfficeFavoritesCommand::GetIcon()
{
	return IconLoader::Get()->LoadExtensionIcon(in->mItem.GetExtension());
}

launcherapp::core::Command*
OfficeFavoritesCommand::Clone()
{
	return new OfficeFavoritesCommand(in->mItem);
}

// メニューの項目数を取得する
int OfficeFavoritesCommand::GetMenuItemCount()
{
	return 2;
}

// メニューに対応するアクションを取得する
bool OfficeFavoritesCommand::GetMenuItem(int index, Action** action)
{
	if (index == 0) {
		return GetAction(HOTKEY_ATTR(0, VK_RETURN), action);
	}
	else if (index == 1 && in->mItem.IsURL() == false) {
		return GetAction(HOTKEY_ATTR(MOD_CONTROL, VK_RETURN), action);
	}
	else if (index == 2 && in->mItem.IsURL() == false) {
		return GetAction(HOTKEY_ATTR(MOD_ALT, VK_RETURN), action);
	}
	return false;
}

bool OfficeFavoritesCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (AdhocCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	return false;
}

// 表示名("お気に入り(アプリ名)")を取得する
CString OfficeFavoritesCommand::TypeDisplayName(const CString& appName)
{
	CString fmt((LPCTSTR)IDS_COMMAND_OFFICEFAVORITES);
	CString result;
	result.Format(fmt, (LPCTSTR)appName);
	return result;
}

} // end of namespace office_favorites
} // end of namespace commands
} // end of namespace launcherapp
