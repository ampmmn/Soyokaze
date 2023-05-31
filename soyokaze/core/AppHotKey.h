// AppHotKey ヘッダファイル
#pragma once

#include "AppPreferenceListenerIF.h"
#include <memory>

namespace soyokaze {
namespace core {

class GlobalHotKey;

// アプリ呼び出しキー(ホットキー)の登録・解除を行うクラス
class AppHotKey : public AppPreferenceListenerIF
{
public:
	AppHotKey(HWND targetWnd);
	virtual ~AppHotKey();

public:
	// 設定ファイルから設定値を取得してホットキー登録
	bool Register();
	// 登録解除する
	void Unregister();

	bool IsSameKey(LPARAM lParam);

	// 再登録(登録解除→登録)
	bool Reload();

private:
	static bool LoadKeyConfig(UINT& modifiers, UINT& vk);

	void OnAppFirstBoot() override;
	void OnAppPreferenceUpdated() override;
protected:
	std::unique_ptr<GlobalHotKey> mHotKey;
};

}
}

