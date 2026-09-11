#pragma once

#include "commands/common/AdhocCommandBase.h"

namespace launcherapp {
namespace commands {
namespace shelluri {

class ShellUriCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	/**
	  Shell URIコマンドを初期化する
	*/
	ShellUriCommand();

	/**
	  Shell URIコマンドを破棄する
	*/
	virtual ~ShellUriCommand();

	/**
	  入力されたShell URIをコマンド名として取得する
	  @return 入力されたShell URI
	*/
	CString GetName() override;

	/**
	  入力されたShell URIをコマンドの説明として取得する
	  @return 入力されたShell URI
	*/
	CString GetDescription() override;

	/**
	  コマンド種別の表示名を取得する
	  @return コマンド種別の表示名
	*/
	CString GetTypeDisplayName() override;

	/**
	  Shell URIを実行するアクションを作成する
	  @param[in] hotkeyAttr ホットキー属性
	  @param[out] action 作成したアクション
	  @return true:アクションを作成した  false:作成しない
	*/
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;

	/**
	  Shell URIコマンドのアイコンを取得する
	  @return コマンドのアイコン
	*/
	HICON GetIcon() override;

	/**
	  入力パターンがShell URIに一致するか判定する
	  @param[in] pattern 入力パターン
	  @return 一致度
	*/
	int Match(Pattern* pattern) override;

	/**
	  Shell URIコマンドを複製する
	  @return 複製したコマンド
	*/
	launcherapp::core::Command* Clone() override;

	/**
	  コマンド種別の表示名を取得する
	  @return コマンド種別の表示名
	*/
	static CString TypeDisplayName();

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(ShellUriCommand)
};

} // end of namespace shelluri
} // end of namespace commands
} // end of namespace launcherapp
