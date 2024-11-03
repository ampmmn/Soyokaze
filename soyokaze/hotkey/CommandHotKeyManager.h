#pragma once

#include <memory>
#include <vector>
#include "hotkey/CommandHotKeyHandlerIF.h"
#include "hotkey/CommandHotKeyAttribute.h"
#include "hotkey/CommandHotKeyMappings.h"

namespace launcherapp {
namespace core {


class CommandHotKeyManager
{
private:
	CommandHotKeyManager();
	~CommandHotKeyManager();

public:
	enum { 
		ID_LOCAL_START = 32771,
		ID_LOCAL_END   = ID_LOCAL_START + 512,
		ID_GLOBAL_START = 0xB31E + 1
	};

public:
	// インスタンスを取得する
	static CommandHotKeyManager* GetInstance();

	// グローバルホットキーのイベント受け取り先となるウインドウを設定する
	HWND SetReceiverWindow(HWND hwnd);

	// メッセージ内容によりハンドラを呼ぶ
	bool TryCallLocalHotKeyHander(MSG* msg);

	// 実行
	void InvokeLocalHandler(UINT id);
	void InvokeGlobalHandler(LPARAM lp);

	// 名前からホットキーを取得
	bool HasKeyBinding(const CString& name, CommandHotKeyAttribute* keyPtr);
	bool GetKeyBinding(const CString& name, CommandHotKeyAttribute* keyPtr);

	// ホットキーに関連付けられたものがあるかをチェック
	bool HasKeyBinding(const CommandHotKeyAttribute& key);

	// 登録
	bool Register(void* owner, CommandHotKeyHandler* handler, const CommandHotKeyAttribute& key);
	// 登録解除(ハンドラオブジェクトがわかっている場合利用可能)
	bool Unregister(CommandHotKeyHandler* handler);
	// 登録された要素数を取得
	int GetItemCount();

	void GetMappings(CommandHotKeyMappings& keyMap);

	// 登録された要素を全削除
	void Clear(void* owner);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace core
} // end of namespace launcherapp

