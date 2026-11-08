#pragma once

#include "commands/common/UserCommandProviderBase.h"

namespace launcherapp {
namespace commands {
namespace output_preview {

class OutputPreviewCommandProvider : public launcherapp::commands::common::UserCommandProviderBase
{
private:
	/**
	  Providerを初期化する
	*/
	OutputPreviewCommandProvider();
	/**
	  Providerを破棄する
	*/
	~OutputPreviewCommandProvider() override;

public:
	/**
	  Provider名を取得する
	  @return Provider名
	*/
	CString GetName() override;
	/**
	  コマンド種別の表示名を取得する
	  @return 表示名
	*/
	CString GetDisplayName() override;
	/**
	  コマンド種別の説明を取得する
	  @return 説明
	*/
	CString GetDescription() override;
	/**
	  新しい実行結果確認コマンドを作成する
	  @param[in] param 作成時のパラメーター
	  @return 作成に成功した場合はtrue
	*/
	bool NewDialog(Parameter* param) override;
	/**
	  Providerの優先順位を取得する
	  @return 優先順位
	*/
	uint32_t GetOrder() const override;
	/**
	  管理するコマンド種別の表示名を列挙する
	  @param[out] displayNames 表示名一覧
	  @return 列挙した件数
	*/
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;
	/**
	  実行結果表示用の一時コマンドを問い合わせる
	  @param[in] pattern 入力パターン
	  @param[out] commands 一致したコマンド
	*/
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands) override;
	/**
	  コマンド読み込み前の初期化を行う
	*/
	void OnBeforeLoad() override;
	/**
	  保存データからコマンドを復元する
	  @param[in] entry 保存データ
	  @param[out] command 復元したコマンド
	  @return 復元に成功した場合はtrue
	*/
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;

	DECLARE_COMMANDPROVIDER(OutputPreviewCommandProvider)
};

}
}
}
