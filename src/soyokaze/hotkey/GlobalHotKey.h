// あ
#pragma once

namespace launcherapp {
namespace core {

class GlobalHotKey
{
public:
	GlobalHotKey(HWND targetWnd);
	~GlobalHotKey();

public:
	// 設定ファイルから設定値を取得してホットキー登録
	bool Register(UINT id, UINT mod, UINT vk);
	// 登録解除する
	void Unregister();

	bool IsSameKey(LPARAM lParam);

	UINT GetID() const;

	CString ToString() const;

protected:
	// ホットキー登録先ウインドウ
	HWND mTargetWnd;
	// 修飾キーを表すフラグ
	UINT mModifiers;
	// 仮想キーコード
	UINT mVirtualKey;
	// ホットキー登録時に割り当てるID
	UINT mID;
};

}
}

