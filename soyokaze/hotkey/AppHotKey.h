// あ
#pragma once

#include "setting/AppPreferenceListenerIF.h"
#include <memory>

namespace launcherapp {
namespace core {

class GlobalHotKey;

// ランチャー呼び出しキー(ホットキー)の登録・解除を行うクラス
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

	CString ToString() const;

private:
	void OnAppFirstBoot() override;
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override;
	void OnAppExit() override;
protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

}
}

