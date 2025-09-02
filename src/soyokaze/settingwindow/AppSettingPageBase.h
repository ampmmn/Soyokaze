#pragma once

#include "settingwindow/AppSettingPageIF.h"

namespace launcherapp {
namespace settingwindow {

class AppSettingPageBase :
	virtual public AppSettingPageIF
{
public:
	AppSettingPageBase(LPCTSTR pagePath, LPCTSTR name);
	~AppSettingPageBase();

	// ウインドウを作成する
	bool Create(HWND parentWindow) override;
	// ウインドウハンドルを取得する
	HWND GetHwnd() override;
	// ページのパス(\\区切りで階層を表現する)
	CString GetPagePath() override;
	// ページ名
	CString GetName() override;
	// 同じ親の中で表示する順序(低いほど先に表示)
	int GetOrder() override;
	// 
	bool OnEnterSettings() override;
	// ページがアクティブになるときに呼ばれる
	bool OnSetActive() override;
	// ページが非アクティブになるときに呼ばれる
	bool OnKillActive() override;
	//
	void OnOKCall() override;
	//
	void SetParam(void* param) override;

	// ページに関連付けられたヘルプページIDを取得する
	bool GetHelpPageId(String& helpPageId) override;

	// インスタンスを複製する
	AppSettingPageIF* Clone() override;

	bool QueryInterface(const launcherapp::core:: IFID& ifid, void** cmd) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

protected:
	void* GetParam();

protected:
	CString mPagePath;
	CString mPageName;
	void* mParam;
	uint32_t mRefCount;
};


}  // end of namespace settingwindow
}  // end of namespace launcherapp

