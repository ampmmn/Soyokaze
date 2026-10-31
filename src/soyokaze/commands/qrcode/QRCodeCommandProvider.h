#pragma once

#include "commands/common/AdhocCommandProviderBase.h"
#include "commands/core/CommandProviderIF.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace qrcode {

/**
  QRコード生成の一時コマンドを提供する
*/
class QRCodeCommandProvider :
	public launcherapp::commands::common::AdhocCommandProviderBase
{
private:
	/** プロバイダーを生成する */
	QRCodeCommandProvider();
	/** プロバイダーを破棄する */
	~QRCodeCommandProvider() override;

public:
	/** @return プロバイダー名 */
	CString GetName() override;
	/** アプリケーション設定を読み込み、候補準備を行う */
	void PrepareAdhocCommands() override;
	/**
	  入力されたキーワードと文字列からQRコード候補を作成する
	  @param[in] pattern 入力パターン
	  @param[out] commands 追加するコマンド候補一覧
	*/
	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& commands) override;
	/**
	  このプロバイダーが提供するコマンド種別を列挙する
	  @return 列挙したコマンド種別数
	  @param[out] displayNames コマンド種別名一覧
	*/
	uint32_t EnumCommandDisplayNames(std::vector<CString>& displayNames) override;

	DECLARE_COMMANDPROVIDER(QRCodeCommandProvider)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
