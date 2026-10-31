#pragma once

#include "features/main/preview/PreviewWindow.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace qrcode {

/**
  QRコード画像を候補プレビューとして表示する
*/
class QRCodePreviewWindow : public launcherapp::mainwindow::preview::PreviewWindow
{
private:
	/** プレビューウインドウを初期化する */
	QRCodePreviewWindow();
	/** プレビューウインドウを解放する */
	~QRCodePreviewWindow() override;

public:
	/** @return QRコードプレビューの共有インスタンス */
	static QRCodePreviewWindow* Get();

	/** プレビューを表示する */
	void Enter() override;
	/** プレビューを非表示にする */
	void Leave() override;
	/** @return プレビューウインドウのハンドル */
	HWND GetHwnd() override;

	/**
	  プレビューに表示するQRコードを設定する
	  @return true:成功  false:生成失敗
	  @param[in] text QRコードにする文字列
	  @param[in] errorCorrectionLevel 誤り訂正レベル
	*/
	bool SetCode(const CString& text, int errorCorrectionLevel);

	/**
	  プレビューウインドウを破棄する
	*/
	void Destroy();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
