#pragma once

#include <memory>
#include <vector>
#include "core/CommandHotKeyHandlerIF.h"
#include "HotKeyAttribute.h"  // ToDo: coreに移動
#include "CommandHotKeyMappings.h"

namespace soyokaze {
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

	// アクセラレータ取得
	HACCEL GetAccelerator();
	// 実行
	void InvokeLocalHandler(UINT id);
	void InvokeGlobalHandler(LPARAM lp);

	// 名前からホットキーを取得
	bool HasKeyBinding(const CString& name, HOTKEY_ATTR* keyPtr, bool* isGlobalPtr);

	// ホットキーに関連付けられたものがあるかをチェック
	bool HasKeyBinding(const HOTKEY_ATTR& key, bool* isGlobalPtr = nullptr);

	// 登録
	bool Register(CommandHotKeyHandler* handler, const HOTKEY_ATTR& key, bool isGlobal);
	// 登録された要素数を取得
	int GetItemCount();
	// 登録された要素の情報を取得する
	bool GetItem(int index, CommandHotKeyHandler** handler, HOTKEY_ATTR* keyPtr = nullptr, bool* isGlobalPtr = nullptr);

	void GetMappings(CommandHotKeyMappings& keyMap);

	// 登録された要素を全削除
	void Clear();

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

} // end of namespace core
} // end of namespace soyokaze

