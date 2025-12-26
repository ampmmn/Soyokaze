#pragma once

#include "core/UnknownIF.h"
#include "hotkey/HotKeyAttribute.h"

namespace launcherapp { namespace commands { namespace core {

// 追加のアクションに対するホットキー設定を持つインタフェース
// コマンドがこのインターフェースを実装すると、Enterキー以外のキーバインドでアクションを実行できる
class ExtraActionHotKeySettings : virtual public launcherapp::core::UnknownIF
{
public:
	// ホットキー設定の数を取得
	virtual int GetHotKeyCount() = 0;
	// ホットキー設定を取得
	virtual bool GetHotKeyAttribute(int index, HOTKEY_ATTR& hotkeyAttr) = 0;
};


}}} // end of namespace launcherapp::commands::core

