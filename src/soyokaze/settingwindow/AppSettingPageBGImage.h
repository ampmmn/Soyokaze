#pragma once

#include "settingwindow/AppSettingPageBase.h"
#include "settingwindow/AppSettingPageRepository.h"
#include <memory>

class AppSettingPageBGImage :
	virtual public launcherapp::settingwindow::AppSettingPageBase
{
public:
	AppSettingPageBGImage();
	~AppSettingPageBGImage();

	// ウインドウを作成する
	bool Create(HWND parentWindow) override;
	// ウインドウハンドルを取得する
	HWND GetHwnd() override;
	// 同じ親の中で表示する順序(低いほど先に表示)
	int GetOrder() override;
	// 設定値を読み込む
	bool OnEnterSettings() override;
	// ページがアクティブになるときに呼ばれる
	bool OnSetActive() override;
	// ページが非アクティブになるときに呼ばれる
	bool OnKillActive() override;
	// 設定値を保存する
	void OnOKCall() override;

	// ページに関連付けられたヘルプページIDを取得する
	bool GetHelpPageId(String& helpPageId) override;

	// インスタンスを複製する
	AppSettingPageIF* Clone() override { return new AppSettingPageBGImage(); }

	DECLARE_APPSETTINGPAGE(AppSettingPageBGImage)
private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};
