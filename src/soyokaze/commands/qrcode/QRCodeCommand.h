#pragma once

#include "commands/common/AdhocCommandBase.h"
#include "commands/core/PreviewProvider.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace qrcode {

/**
  QRコードの生成候補を表す一時コマンド
*/
class QRCodeCommand :
	virtual public launcherapp::commands::common::AdhocCommandBase,
	virtual public launcherapp::core::PreviewProvider
{
public:
	/**
	  入力文字列と誤り訂正レベルを保持する
	  @param[in] prefix トリガーワード
	  @param[in] text QRコードに格納する文字列
	  @param[in] errorCorrectionLevel 誤り訂正レベル
	*/
	QRCodeCommand(const CString& prefix, const CString& text, int errorCorrectionLevel);
	/**
	  コマンドの内部データを解放する
	*/
	~QRCodeCommand() override;

	/** @return コマンド候補に表示する文字列 */
	CString GetName() override;
	/** @return コマンド種別の表示名 */
	CString GetTypeDisplayName() override;
	/**
	  EnterまたはCtrl+Enterに対応するアクションを取得する
	  @return true:対応するアクションを取得  false:未対応の修飾キー
	  @param[in] hotkeyAttr 押下キー属性
	  @param[out] action 生成したアクション
	*/
	bool GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action) override;

	HICON GetIcon() override;

	/** @return コマンドの複製 */
	launcherapp::core::Command* Clone() override;
	/** @return QRコードプレビューウインドウ */
	launcherapp::mainwindow::preview::PreviewWindow* FetchPreviewWindow() override;
	/**
	  コマンドまたはプレビュー提供インターフェースを取得する
	  @return true:対応するインターフェースを取得  false:未対応
	  @param[in] ifid インターフェース識別子
	  @param[out] cmd 取得したインターフェース
	*/
	bool QueryInterface(const launcherapp::core::IFID& ifid, void** cmd) override;

	DECLARE_ADHOCCOMMAND_UNKNOWNIF(QRCodeCommand)

public:
	/** @return コマンド種別の表示名 */
	static CString TypeDisplayName();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
