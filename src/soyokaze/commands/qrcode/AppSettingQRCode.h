#pragma once

#include "features/appsettings/AppSettingPageBase.h"
#include "features/appsettings/AppSettingPageRepository.h"
#include <memory>

namespace launcherapp {
namespace commands {
namespace qrcode {

/**
  QRコード生成のアプリケーション設定ページ
*/
class AppSettingQRCode : public launcherapp::settingwindow::AppSettingPageBase
{
public:
	/** 設定ページを初期化する */
	AppSettingQRCode();
	/** 設定ページの内部リソースを解放する */
	~AppSettingQRCode() override;

	/**
	  設定ページのウインドウを作成する
	  @return true:成功  false:失敗
	  @param[in] parentWindow 親ウインドウ
	*/
	bool Create(HWND parentWindow) override;
	/** @return 設定ページのウインドウハンドル */
	HWND GetHwnd() override;
	/** @return 同じ階層内での表示順 */
	int GetOrder() override;
	/** @return true:設定を読み込んだ  false:失敗 */
	bool OnEnterSettings() override;
	/** @return true:ページをアクティブにできる */
	bool OnSetActive() override;
	/** @return true:ページを離れられる  false:入力値が不正 */
	bool OnKillActive() override;
	/** 設定値を保存する */
	void OnOKCall() override;
	/**
	  このページに対応するマニュアル識別子を取得する
	  @return true:識別子を取得
	  @param[out] helpPageId マニュアル識別子
	*/
	bool GetHelpPageId(String& helpPageId) override;
	/** @return 設定ページの複製 */
	AppSettingPageIF* Clone() override { return new AppSettingQRCode(); }

	DECLARE_APPSETTINGPAGE(AppSettingQRCode)

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // qrcode名前空間
} // commands名前空間
} // launcherapp名前空間
