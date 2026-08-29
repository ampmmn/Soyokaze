#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/plugin/PluginModule.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace plugin {

class PluginCommand : public launcherapp::commands::common::AdhocCommandBase
{
public:
	/**
	  プラグインの検索結果からコマンドを生成する
	  @param[in] module プラグインモジュール
	  @param[in] match プラグインの検索結果ハンドル
	  @param[in] index 検索結果内のコマンド番号
	  @param[in] name コマンド名
	  @param[in] description コマンドの説明
	*/
	PluginCommand(const PluginModulePtr& module, const PluginMatchPtr& match, int index,
	              const CString& name, const CString& description);
	/**
	  プラグインコマンドを破棄する
	*/
	~PluginCommand() override;

	/**
	  コマンド種別の表示名を取得する
	  @return コマンド種別の表示名
	*/
	CString GetTypeDisplayName() override;
	/**
	  コマンドを実行できるか確認する
	  @param[out] reasonMsg 実行できない場合の理由
	  @return true:実行可能 false:実行不可
	*/
	bool CanExecute(String* reasonMsg) override;
	/**
	  コマンド実行用のアクションを生成する
	  @param[in] hotkeyAttr 起動時に指定されたホットキー属性
	  @param[out] action 生成したアクション
	  @return true:生成成功 false:生成不可
	*/
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;
	/**
	  コマンドのアイコンを取得する
	  @return アイコンハンドル
	*/
	HICON GetIcon() override;
	/**
	  プラグインコマンドを複製する
	  @return 複製されたコマンド
	*/
	Command* Clone() override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(PluginCommand)

private:
	PluginModulePtr mModule;
	PluginMatchPtr mMatch;
	int mIndex{0};
	CString mTypeDisplayName;
	HICON mIcon{nullptr};
};

} // namespace plugin
} // namespace commands
} // namespace launcherapp
