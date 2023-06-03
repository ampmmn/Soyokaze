#include "pch.h"
#include "CommandHotKeyManager.h"
#include "core/GlobalHotKey.h"
#include <set>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace core {

static const int ID_SOYOKAZE_COMMAND_HOTKEY = 0xB31E + 1;

struct CommandHotKeyManager::PImpl
{
	struct ITEM
	{
		ITEM() : mHandlerPtr(nullptr), mIsGlobal(false), mGlobalHotKey(nullptr)
		{
		}

		// ホットキーから呼び出すハンドラオブジェクト
		CommandHotKeyHandler* mHandlerPtr;

		// グローバルなホットキーか
		bool mIsGlobal;

		// グローバルホットキーの場合
		std::unique_ptr<GlobalHotKey> mGlobalHotKey;
	};

	UINT GetHotKeyID();

	HACCEL mAccel;
	bool mIsChanged;

	// 関連付けられた名前からキー割り当てを引くためのmap
	std::map<CString, HOTKEY_ATTR> mNameKeyMap;
	// キー割り当てから要素を引くためのmap
	std::map<HOTKEY_ATTR, ITEM> mKeyItemMap;
	std::map<UINT, CommandHotKeyHandler*> mLocalHandlerMap;
};

UINT CommandHotKeyManager::PImpl::GetHotKeyID()
{
	std::set<UINT> sets;

	for(auto& elem : mKeyItemMap) {
		const auto& item = elem.second;
		if (item.mGlobalHotKey.get() == nullptr) {
			continue;
		}
		sets.insert(item.mGlobalHotKey->GetID());
	}

	UINT curId = ID_GLOBAL_START;
	for (UINT id : sets) {
		if (id != curId) {
			break;
		}
		curId++;
	}
	return curId;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



CommandHotKeyManager::CommandHotKeyManager() : in(new PImpl)
{
	in->mAccel = nullptr;
	in->mIsChanged = false;
}

CommandHotKeyManager::~CommandHotKeyManager()
{
	Clear();
}

CommandHotKeyManager*
CommandHotKeyManager::GetInstance()
{
	static CommandHotKeyManager inst;
	return &inst;
}


// アクセラレータ取得
HACCEL CommandHotKeyManager::GetAccelerator()
{
	// すでにアクセラレータテーブル作成済で、かつ、設定に変更がなければ再利用する
	if (in->mIsChanged == false) {
		return in->mAccel;
	}

	// 変更かあれば再作成するのでいったん破棄
	if (in->mAccel) {
		DestroyAcceleratorTable(in->mAccel);
		in->mAccel = nullptr;
	}

	// テーブル情報が空の場合はnull
	if (in->mKeyItemMap.empty()) {
		return nullptr;
	}

	// 再作成
	in->mLocalHandlerMap.clear();
	std::vector<ACCEL> accels;
	accels.reserve(in->mKeyItemMap.size());

	UINT id = ID_LOCAL_START;
	for (auto& elem : in->mKeyItemMap) {

		const HOTKEY_ATTR& hotKeyAttr = elem.first;
		auto& item = elem.second;

		if (item.mIsGlobal) {
			// グローバルホットキーはアクセラレータテーブルでは扱わない
			continue;
		}

		ACCEL accel;
		hotKeyAttr.GetAccel(accel);
		accel.cmd = id;

		accels.push_back(accel);
		in->mLocalHandlerMap[id] = item.mHandlerPtr;
		id++;
	}

	if (accels.empty()) {
		return nullptr;
	}

	in->mAccel = CreateAcceleratorTable(&accels.front(), (int)accels.size());
	in->mIsChanged = false;
	return in->mAccel;
}

void CommandHotKeyManager::InvokeLocalHandler(UINT id)
{
	auto itFind = in->mLocalHandlerMap.find(id);
	if (itFind == in->mLocalHandlerMap.end()) {
		return ;
	}
	auto handlerPtr = itFind->second;
	ASSERT(handlerPtr);
	handlerPtr->Invoke();
}

void CommandHotKeyManager::InvokeGlobalHandler(LPARAM lp)
{
	HOTKEY_ATTR attr(LOWORD(lp), HIWORD(lp));
	auto itFind = in->mKeyItemMap.find(attr);
	if (itFind == in->mKeyItemMap.end()) {
		return ;
	}

	PImpl::ITEM& item = itFind->second;
	ASSERT(item.mIsGlobal);
	item.mHandlerPtr->Invoke();
}

// 名前からホットキーを取得
bool CommandHotKeyManager::HasKeyBinding(
	const CString& name,
	HOTKEY_ATTR* keyPtr,
	bool* isGlobalPtr
)
{
	auto itFind = in->mNameKeyMap.find(name);
	if (itFind == in->mNameKeyMap.end()) {
		return false;
	}

	HOTKEY_ATTR attr = itFind->second;
	if (keyPtr) {
		*keyPtr = attr;
	}
	if (isGlobalPtr) {
		auto itFind2 = in->mKeyItemMap.find(attr);
		*isGlobalPtr = itFind2->second.mIsGlobal;
	}

	return true;
}

// ホットキーに関連付けられたものがあるかをチェック
bool CommandHotKeyManager::HasKeyBinding(
	const HOTKEY_ATTR& key,
	bool* isGlobalPtr
)
{
	auto itFind = in->mKeyItemMap.find(key);
	if (itFind == in->mKeyItemMap.end()) {
		return false;
	}

	if (isGlobalPtr == nullptr) {
		return true;
	}

	*isGlobalPtr = itFind->second.mIsGlobal;
	return true;
}

bool CommandHotKeyManager::Register(
	CommandHotKeyHandler* handler,
	const HOTKEY_ATTR& key,
	bool isGlobal
)
{
	ASSERT(handler);

	auto itFind = in->mKeyItemMap.find(key);
	if (itFind == in->mKeyItemMap.end()) {
		auto p = in->mKeyItemMap.insert(std::make_pair(key, PImpl::ITEM()));
		itFind = p.first;
	}


	PImpl::ITEM& item = itFind->second;

	// グローバルホットキーの処理(使えないキー場合はエラーを返す)
	if (isGlobal) {
		std::unique_ptr<GlobalHotKey> hotKey(new GlobalHotKey(AfxGetMainWnd()->GetSafeHwnd()));
		if (hotKey->Register(in->GetHotKeyID(), key.GetModifiers(), key.GetVKCode()) == false) {
			// 登録失敗
			return false;
		}
		item.mGlobalHotKey.swap(hotKey);
	}
	else {
		item.mGlobalHotKey.reset();
	}

	// Delete older one.
	delete item.mHandlerPtr;
	item.mIsGlobal = isGlobal;
	item.mHandlerPtr = handler;

	in->mIsChanged = true;

	in->mNameKeyMap[handler->GetDisplayName()] = key;

	return true;
}

int CommandHotKeyManager::GetItemCount()
{
	return (int)in->mKeyItemMap.size();
}

bool CommandHotKeyManager::GetItem(int index, CommandHotKeyHandler** handler, HOTKEY_ATTR* keyPtr, bool * isGlobalPtr)
{
	if (index >= in->mKeyItemMap.size()) {
		return false;
	}
	auto& it = in->mKeyItemMap.begin();
	std::advance(it, index);

	if (handler) {
		*handler = it->second.mHandlerPtr;
	}
	if (keyPtr) {
		*keyPtr = (it->first);
	}
	if (isGlobalPtr) {
		*isGlobalPtr = it->second.mIsGlobal;
	}

	return true;
}

void CommandHotKeyManager::GetMappings(CommandHotKeyMappings& keyMap)
{
	CommandHotKeyMappings tmp;
	for (auto elem : in->mNameKeyMap) {

		const HOTKEY_ATTR& attr = elem.second;

		auto& itKeyItem = in->mKeyItemMap.find(attr);
		PImpl::ITEM& item = itKeyItem->second;

		tmp.AddItem(elem.first, attr, item.mIsGlobal);
	}
	keyMap.Swap(tmp);
}

// 登録された要素を全削除
void CommandHotKeyManager::Clear()
{
	in->mNameKeyMap.clear();

	CWnd* wnd = AfxGetMainWnd();
	HWND hwnd = wnd ? wnd->GetSafeHwnd() : NULL;

	for (auto& elem : in->mKeyItemMap) {
		auto& item = elem.second;
		delete item.mHandlerPtr;

		if (IsWindow(hwnd) && item.mIsGlobal && item.mGlobalHotKey.get() != nullptr) {
			item.mGlobalHotKey.reset();
		}
	}
	in->mKeyItemMap.clear();
	in->mLocalHandlerMap.clear();
}

} // end of namespace core
} // end of namespace soyokaze

