#include "pch.h"
#include "CommandHotKeyManager.h"
#include "hotkey/GlobalHotKey.h"
#include <set>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace core {

struct NameMapValue {
	NameMapValue() = default;

	NameMapValue(void* owner, const HOTKEY_ATTR& attr) : mOwner(owner), mAttr(attr)
	{
	}

	NameMapValue(const NameMapValue&) = default;
	HOTKEY_ATTR mAttr;
	void* mOwner;
};

using NameMap = std::map<CString, NameMapValue>;

static const int ID_SOYOKAZE_COMMAND_HOTKEY = 0xB31E + 1;

struct CommandHotKeyManager::PImpl
{
	struct ITEM
	{
		ITEM() : mIsGlobal(false), mGlobalHotKey(nullptr)
		{
		}

		// ホットキーから呼び出すハンドラオブジェクト
		std::unique_ptr<CommandHotKeyHandler> mHandlerPtr;

		// グローバルなホットキーか
		bool mIsGlobal;

		// グローバルホットキーの場合
		std::unique_ptr<GlobalHotKey> mGlobalHotKey;
	};

	UINT GetHotKeyID();

	CCriticalSection mCS;

	HWND mReceiverWindow;

	HACCEL mAccel;
	bool mIsChanged;

	// 関連付けられた名前からキー割り当てを引くためのmap
	NameMap mNameKeyMap;
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



CommandHotKeyManager::CommandHotKeyManager() : in(std::make_unique<PImpl>())
{
	in->mReceiverWindow = nullptr;
	in->mAccel = nullptr;
	in->mIsChanged = false;
}

CommandHotKeyManager::~CommandHotKeyManager()
{
}

CommandHotKeyManager*
CommandHotKeyManager::GetInstance()
{
	static CommandHotKeyManager inst;
	return &inst;
}

// グローバルホットキーのイベント受け取り先となるウインドウを設定する
HWND CommandHotKeyManager::SetReceiverWindow(HWND hwnd)
{
	CSingleLock sl(&in->mCS, TRUE);

	HWND orgWindow = in->mReceiverWindow;
	in->mReceiverWindow = hwnd;

	return orgWindow;
}

// アクセラレータ取得
HACCEL CommandHotKeyManager::GetAccelerator()
{
	// 頻繁に呼ばれるのでここではログをださない
	// SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&in->mCS, TRUE);

	// すでにアクセラレータテーブル作成済で、かつ、設定に変更がなければ再利用する
	if (in->mIsChanged == false) {
		// 頻繁に呼ばれるのでここではログをださない
		//SPDLOG_DEBUG(_T("Reuse HACCEL."));
		return in->mAccel;
	}

	// 変更かあれば再作成するのでいったん破棄
	if (in->mAccel) {
		DestroyAcceleratorTable(in->mAccel);
		in->mAccel = nullptr;

		SPDLOG_DEBUG(_T("Destroy HACCEL."));
	}

	// テーブル情報が空の場合はnull
	if (in->mKeyItemMap.empty()) {
		SPDLOG_DEBUG(_T("mKeyItemMap is empty. HACCEL will be nullptr."));
		return nullptr;
	}

	// 再作成
	in->mLocalHandlerMap.clear();
	std::vector<ACCEL> accels;
	accels.reserve(in->mKeyItemMap.size());

	// ローカルホットキー用を実行できるようにするため、
	// キーアクセラレータに登録するテーブル生成
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
		in->mLocalHandlerMap[id] = item.mHandlerPtr.get();

		SPDLOG_DEBUG(_T("local key entry name:{0} id:{1}"), (LPCTSTR)item.mHandlerPtr->GetDisplayName(), id);
		id++;
	}

	SPDLOG_DEBUG(_T("number of local handler : {}"), (int)(id - ID_LOCAL_START));

	if (accels.empty()) {
		SPDLOG_DEBUG(_T("HACCEL is empty."));
		return nullptr;
	}

	in->mAccel = CreateAcceleratorTable(&accels.front(), (int)accels.size());
	in->mIsChanged = false;

	SPDLOG_DEBUG(_T("HACCEL for local key handler created. result:{}"), (in->mAccel != nullptr));

	return in->mAccel;
}

void CommandHotKeyManager::InvokeLocalHandler(UINT id)
{
	SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&in->mCS, TRUE);

	auto itFind = in->mLocalHandlerMap.find(id);
	if (itFind == in->mLocalHandlerMap.end()) {
		SPDLOG_WARN(_T("Handler does not found."));
		return ;
	}
	auto handlerPtr = itFind->second;

	sl.Unlock();
	ASSERT(handlerPtr);
	handlerPtr->Invoke();
}

void CommandHotKeyManager::InvokeGlobalHandler(LPARAM lp)
{
	SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&in->mCS, TRUE);

	HOTKEY_ATTR attr(LOWORD(lp), HIWORD(lp));

	auto itFind = in->mKeyItemMap.find(attr);
	if (itFind == in->mKeyItemMap.end()) {
		SPDLOG_WARN(_T("Handler does not found."));
		return ;
	}

	PImpl::ITEM& item = itFind->second;
	sl.Unlock();

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
	SPDLOG_DEBUG(_T("args name:{0}"), (LPCTSTR)name);

	CSingleLock sl(&in->mCS, TRUE);

	auto itFind = in->mNameKeyMap.find(name);
	if (itFind == in->mNameKeyMap.end()) {
		spdlog::debug(_T("key bind does not exist. name:{0}"), (LPCTSTR)name);
		return false;
	}
	spdlog::debug(_T("key bind exists. name:{0}"), (LPCTSTR)name);

	HOTKEY_ATTR attr = itFind->second.mAttr;
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
	SPDLOG_DEBUG(_T("args Modifier:{0} VKCode:{1}"), key.GetModifiers(), key.GetVKCode());

	CSingleLock sl(&in->mCS, TRUE);

	auto itFind = in->mKeyItemMap.find(key);
	if (itFind == in->mKeyItemMap.end()) {
		spdlog::debug(_T("key bind does not exist."));
		return false;
	}
	spdlog::debug(_T("key bind exists."));

	if (isGlobalPtr == nullptr) {
		spdlog::debug(_T("isGlobalPtr is nullptr"));
		return true;
	}

	*isGlobalPtr = itFind->second.mIsGlobal;
	spdlog::debug(_T("isGlobal:{0}"), *isGlobalPtr);
	return true;
}

bool CommandHotKeyManager::Register(
	void* owner,
	CommandHotKeyHandler* handler,
	const HOTKEY_ATTR& key,
	bool isGlobal
)
{
	SPDLOG_DEBUG(_T("args name:{0} Modifier:{1} VKCode:{2} isGlobal:{3}"), (LPCTSTR)handler->GetDisplayName(), key.GetModifiers(), key.GetVKCode(), isGlobal);

	if (key.GetVKCode() == 0) {
		SPDLOG_DEBUG(_T("ignored VKCode=0"));
		return false;
	}

	CSingleLock sl(&in->mCS, TRUE);

	ASSERT(handler);

	auto itFind = in->mKeyItemMap.find(key);
	if (itFind == in->mKeyItemMap.end()) {
		auto p = in->mKeyItemMap.insert(std::make_pair(key, PImpl::ITEM()));
		itFind = p.first;
	}


	PImpl::ITEM& item = itFind->second;

	// グローバルホットキーの処理(使えないキー場合はエラーを返す)
	if (isGlobal) {
		HWND hwnd = in->mReceiverWindow;
		ASSERT(hwnd);
		auto hotKey = std::make_unique<GlobalHotKey>(hwnd);
		auto hotkeyId = in->GetHotKeyID();
		if (hotKey->Register(hotkeyId, key.GetModifiers(), key.GetVKCode()) == false) {
			// 登録失敗
			spdlog::warn(_T("Failed to register global key. HotKeyId:{0} Modifier:{1} VKCode:{2}"),
			                hotkeyId, key.GetModifiers(), key.GetVKCode());
			return false;
		}
		item.mGlobalHotKey.swap(hotKey);

		spdlog::info(_T("A global key registered. HotKeyId:{0} Modifier:{1} VKCode:{2}"),
			           hotkeyId, key.GetModifiers(), key.GetVKCode());
	}
	else {
		item.mGlobalHotKey.reset();

		spdlog::info(_T("A local key registered. Modifier:{1} VKCode:{2}"), key.GetModifiers(), key.GetVKCode());
	}

	// Delete older one.
	item.mIsGlobal = isGlobal;
	item.mHandlerPtr.reset(handler);

	in->mIsChanged = true;

	in->mNameKeyMap[handler->GetDisplayName()] = NameMapValue(owner, key);

	return true;
}

// 登録解除(ハンドラオブジェクトがわかっている場合利用可能)
bool CommandHotKeyManager::Unregister(CommandHotKeyHandler* handler)
{
	SPDLOG_DEBUG(_T("start"));

	for (auto it = in->mKeyItemMap.begin(); it != in->mKeyItemMap.end(); ++it) {
		const auto& item = it->second;
		if (handler != item.mHandlerPtr.get()) {
			continue;
		}

		// eraseするとhandlerも削除されるのでここで名前を保持しておく
		auto name = handler->GetDisplayName();

		in->mKeyItemMap.erase(it);


		auto it2 = in->mNameKeyMap.find(name);
		if (it2 != in->mNameKeyMap.end()) {
			in->mNameKeyMap.erase(it2);
		}
		in->mIsChanged = true;
		return true;
	}

	return false;
}

int CommandHotKeyManager::GetItemCount()
{
	return (int)in->mKeyItemMap.size();
}

bool CommandHotKeyManager::GetItem(int index, CommandHotKeyHandler** handler, HOTKEY_ATTR* keyPtr, bool * isGlobalPtr)
{
	SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&in->mCS, TRUE);

	if (index >= in->mKeyItemMap.size()) {
		SPDLOG_WARN(_T("out of bounds index:{}"), index);
		return false;
	}
	auto& it = in->mKeyItemMap.begin();
	std::advance(it, index);

	if (handler) {
		*handler = it->second.mHandlerPtr.get();
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
	CSingleLock sl(&in->mCS, TRUE);

	CommandHotKeyMappings tmp;
	for (auto& elem : in->mNameKeyMap) {

		const HOTKEY_ATTR& attr = elem.second.mAttr;

		auto& itKeyItem = in->mKeyItemMap.find(attr);
		PImpl::ITEM& item = itKeyItem->second;

		tmp.AddItem(elem.first, attr, item.mIsGlobal);
	}
	keyMap.Swap(tmp);
}

void CommandHotKeyManager::Clear(void* owner)
{
	SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&in->mCS, TRUE);

	HWND hwnd = in->mReceiverWindow;
	ASSERT(hwnd);

	for (auto it = in->mNameKeyMap.begin(); it != in->mNameKeyMap.end();) {
		if (owner != it->second.mOwner) {
			it++;
			continue;
		}

		auto owner = it->second.mOwner;
		auto& attr = it->second.mAttr;

		auto it2 = in->mKeyItemMap.find(attr);
		if (it2 != in->mKeyItemMap.end()) {
			in->mKeyItemMap.erase(it2);
		}

		it =in->mNameKeyMap.erase(it);

		in->mIsChanged = true;
	}
}

} // end of namespace core
} // end of namespace soyokaze

