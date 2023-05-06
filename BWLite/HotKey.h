#pragma once

#include "AppPreferenceListenerIF.h"

// ホットキーの登録・解除を行うクラス
class HotKey : public AppPreferenceListenerIF
{
public:
	HotKey(HWND targetWnd);
	virtual ~HotKey();

public:
	// 設定ファイルから設定値を取得してホットキー登録
	bool Register();
	// 登録解除する
	void Unregister();

	// 再登録(登録解除→登録)
	bool Reload();

	static bool LoadKeyConfig(UINT& modifiers, UINT& vk);

	virtual void OnAppPreferenceUpdated();
protected:
	HWND mTargetWnd;
};

