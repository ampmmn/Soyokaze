
#pragma once

#include "core/UnknownIF.h"

namespace launcherapp {
namespace settingwindow {

class AppSettingPageIF :
	virtual public launcherapp::core::UnknownIF
{
public:
	virtual ~AppSettingPageIF() {}

	// ウインドウを作成する
	virtual bool Create(HWND parentWindow) = 0;
	// ウインドウハンドルを取得する
	virtual HWND GetHwnd() = 0;
	// ページのパス(\\区切りで階層を表現する)
	virtual CString GetPagePath() = 0;
	// ページ名
	virtual CString GetName() = 0;
	// 同じ親の中で表示する順序(低いほど先に表示)
	virtual int GetOrder() = 0;
	// 
	virtual bool OnEnterSettings() = 0;
	// ページがアクティブになるときに呼ばれる
	virtual bool OnSetActive() = 0;
	// ページが非アクティブになるときに呼ばれる
	virtual bool OnKillActive() = 0;
	//
	virtual void OnOKCall() = 0;
	//
	virtual void SetParam(void* param) = 0;

	// ページに関連付けられたヘルプページIDを取得する
	virtual bool GetHelpPageId(String& helpPageId) = 0;

	// インスタンスを複製する
	virtual AppSettingPageIF* Clone() = 0;
};


}  // end of namespace settingwindow
}  // end of namespace launcherapp

