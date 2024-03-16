#pragma once

namespace soyokaze {
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
	HWND mTargetWnd;
	UINT mModifiers;
	UINT mVirtualKey;
	UINT mID;
};

}
}

