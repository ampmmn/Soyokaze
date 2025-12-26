#include "pch.h"
#include "CommandHotKeyManager.h"
#include "hotkey/GlobalHotKey.h"
#include "hotkey/SandSKeyState.h"
#include <set>
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace core {

struct NameMapValue {
	NameMapValue() = default;

	NameMapValue(void* owner, const CommandHotKeyAttribute& attr) : mOwner(owner), mAttr(attr)
	{
	}

	NameMapValue(const NameMapValue&) = default;
	CommandHotKeyAttribute mAttr;
	void* mOwner{nullptr};
};

using NameMap = std::map<CString, NameMapValue>;

struct CommandHotKeyManager::PImpl
{
	struct ITEM
	{
		ITEM() : mIsGlobal(false), mGlobalHotKey(nullptr)
		{
		}

		// ホットキーから呼び出すハンドラオブジェクト
		RefPtr<CommandHotKeyHandler> mHandlerPtr;

		// グローバルなホットキーか
		bool mIsGlobal;

		// グローバルホットキーの場合
		std::unique_ptr<GlobalHotKey> mGlobalHotKey;
	};
	struct SANDS_ITEM
	{
		// ホットキーから呼び出すハンドラオブジェクト
		RefPtr<CommandHotKeyHandler> mHandlerPtr;
	};

	UINT IssueNextGlobalHotKeyID();
	HACCEL GetAccelerator();

	bool RegisterHotKeyAttr(CommandHotKeyHandler* handler, const CommandHotKeyAttribute& key);
	bool RegisterSandSKeyAttr(CommandHotKeyHandler* handler, const CommandHotKeyAttribute& key);

	CCriticalSection mCS;

	HWND mReceiverWindow{nullptr};

	// ローカルホットキー用のアクセラレータ
	HACCEL mAccel{nullptr};
	// 設定変更フラグ
	bool mIsChanged{false};
	bool mIsSandSInvoked{false};

	// 関連付けられた名前からキー割り当てを引くためのmap
	NameMap mNameKeyMap;
	// キー割り当てから要素を引くためのmap
	std::map<HOTKEY_ATTR, ITEM> mKeyItemMap;
	std::map<UINT, CommandHotKeyHandler*> mLocalHandlerMap;

	std::map<SANDSKEY_ATTR, SANDS_ITEM> mSandSItemMap;
};

UINT CommandHotKeyManager::PImpl::IssueNextGlobalHotKeyID()
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

// アクセラレータ取得
HACCEL CommandHotKeyManager::PImpl::GetAccelerator()
{
	// 頻繁に呼ばれるのでここではログをださない
	// SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&mCS, TRUE);

	// すでにアクセラレータテーブル作成済で、かつ、設定に変更がなければ再利用する
	if (mIsChanged == false) {
		// 頻繁に呼ばれるのでここではログをださない
		//SPDLOG_DEBUG(_T("Reuse HACCEL."));
		return mAccel;
	}

	// 変更かあれば再作成するのでいったん破棄
	if (mAccel) {
		DestroyAcceleratorTable(mAccel);
		mAccel = nullptr;

		SPDLOG_DEBUG(_T("Destroy HACCEL."));
	}

	// テーブル情報が空の場合はnull
	if (mKeyItemMap.empty()) {
		SPDLOG_DEBUG(_T("mKeyItemMap is empty. HACCEL will be nullptr."));
		return nullptr;
	}

	// 再作成
	mLocalHandlerMap.clear();
	std::vector<ACCEL> accels;
	accels.reserve(mKeyItemMap.size());

	// ローカルホットキー用を実行できるようにするため、
	// キーアクセラレータに登録するテーブル生成
	UINT id = ID_LOCAL_START;
	for (auto& elem : mKeyItemMap) {

		const HOTKEY_ATTR& hotKeyAttr = elem.first;
		auto& item = elem.second;

		if (item.mIsGlobal) {
			// グローバルホットキーはアクセラレータテーブルでは扱わない
			continue;
		}

		ACCEL accel;
		hotKeyAttr.GetAccel(accel);
		accel.cmd = (WORD)id;

		accels.push_back(accel);
		mLocalHandlerMap[id] = item.mHandlerPtr.get();

		SPDLOG_DEBUG(_T("local key entry name:{0} id:{1}"), (LPCTSTR)item.mHandlerPtr->GetDisplayName(), id);
		id++;
	}

	SPDLOG_DEBUG(_T("number of local handler : {}"), (int)(id - ID_LOCAL_START));

	if (accels.empty()) {
		SPDLOG_DEBUG(_T("HACCEL is empty."));
		return nullptr;
	}

	// アクセラレータを作成しフラグをリセットする
	mAccel = CreateAcceleratorTable(&accels.front(), (int)accels.size());
	mIsChanged = false;

	SPDLOG_DEBUG(_T("HACCEL for local key handler created. result:{}"), (mAccel != nullptr));

	return mAccel;
}

bool CommandHotKeyManager::PImpl::RegisterHotKeyAttr(CommandHotKeyHandler* handler, const CommandHotKeyAttribute& key)
{
	ASSERT(handler);

	HOTKEY_ATTR key_(key.mHotKeyAttr);
	if (key_.IsValid() == false) {
		return false;
	}

	bool isInserted = false;

	auto itFind = mKeyItemMap.find(key_);
	if (itFind == mKeyItemMap.end()) {
		auto p = mKeyItemMap.insert(std::make_pair(key_, PImpl::ITEM()));
		itFind = p.first;
		isInserted = true;
	}


	PImpl::ITEM& item = itFind->second;

	// グローバルホットキーの処理(使えないキー場合はエラーを返す)
	if (key.mIsGlobal) {
		HWND hwnd = mReceiverWindow;
		ASSERT(hwnd);
		auto hotKey = std::make_unique<GlobalHotKey>(hwnd);
		auto hotkeyId = IssueNextGlobalHotKeyID();
		if (hotKey->Register(hotkeyId, key.GetModifiers(), key.GetVKCode()) == false) {
			// 登録失敗
			spdlog::warn(_T("Failed to register global key. HotKeyId:{0} Modifier:{1} VKCode:{2}"),
			                hotkeyId, key.GetModifiers(), key.GetVKCode());
			if (isInserted) {
				mKeyItemMap.erase(itFind);
			}

			return false;
		}
		item.mGlobalHotKey.swap(hotKey);

		spdlog::info(_T("A global key registered. HotKeyId:{0} Modifier:{1} VKCode:{2}"),
			           hotkeyId, key.GetModifiers(), key.GetVKCode());
	}
	else {
		item.mGlobalHotKey.reset();

		spdlog::info(_T("A local key registered. Modifier:{0} VKCode:{1}"), key.GetModifiers(), key.GetVKCode());
	}

	// Delete older one.
	item.mIsGlobal = key.mIsGlobal;
	handler->AddRef();
	item.mHandlerPtr.reset(handler);

	// 変更フラグを立てておく
	mIsChanged = true;

	return true;
}

bool CommandHotKeyManager::PImpl::RegisterSandSKeyAttr(
	 	CommandHotKeyHandler* handler,
	 	const CommandHotKeyAttribute& key
)
{
	ASSERT(handler);

	const auto& attr = key.mSandSKeyAttr;
	if (attr.IsValid() == false) {
		return false;
	}

	auto itFind = mSandSItemMap.find(attr);
	if (itFind == mSandSItemMap.end()) {
		auto p = mSandSItemMap.insert(std::make_pair(attr, PImpl::SANDS_ITEM()));
		itFind = p.first;
	}

	PImpl::SANDS_ITEM& item = itFind->second;

	// Delete older one.
	handler->AddRef();
	item.mHandlerPtr.reset(handler);

	// 変更フラグを立てておく
	mIsChanged = true;

	return true;
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

bool CommandHotKeyManager::TryCallLocalHotKeyHander(MSG* msg)
{

	// ローカルアクセラレータによるホットキー向けの処理
	HACCEL accel = in->GetAccelerator();
	if (accel) {
		// Win32APIに渡す
		if (TranslateAccelerator(in->mReceiverWindow, accel, msg)) {
			return true;
		}
	}

	// SandSによるホットキー向けの処理
	auto sandsState = SandSKeyState::GetInstance();
	if (GetForegroundWindow() != in->mReceiverWindow) {
		// SandSはランチャーウインドウがアクティブなときしか実行しない
		return false;
	}


	for (auto& pa : in->mSandSItemMap) {
		auto& sandsAttr = pa.first;
		auto& sandsItem = pa.second;

		if (sandsState->IsPressed(sandsAttr.GetModifierVKCode(), sandsAttr.GetVKCode()) == false) {
			continue;
		}

		// ハンドラ実行
		auto& handlerPtr = sandsItem.mHandlerPtr;
		handlerPtr->Invoke();
		in->mIsSandSInvoked = true;

		sandsState->Reset();
		return true;
	}

	// 通常のメッセージ処理をする
	return false;
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
	handlerPtr->AddRef();

	sl.Unlock();

	ASSERT(handlerPtr);
	handlerPtr->Invoke();

	handlerPtr->Release();
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

bool CommandHotKeyManager::GetKeyBinding(const CString& name, CommandHotKeyAttribute* keyPtr)
{
	CSingleLock sl(&in->mCS, TRUE);

	auto itFind = in->mNameKeyMap.find(name);
	if (itFind == in->mNameKeyMap.end()) {
		spdlog::debug(_T("key bind does not exist. name:{0}"), (LPCTSTR)name);
		return false;
	}
	spdlog::debug(_T("key bind exists. name:{0}"), (LPCTSTR)name);

	const auto& attr = itFind->second.mAttr;
	if (keyPtr) {
		*keyPtr = attr;
	}
	return true;
}

// ホットキーに関連付けられたものがあるかをチェック
bool CommandHotKeyManager::HasKeyBinding(
	const HOTKEY_ATTR& attr
)
{
	SPDLOG_DEBUG(_T("args Modifier:{0} VKCode:{1}"), attr.GetModifiers(), attr.GetVKCode());

	CSingleLock sl(&in->mCS, TRUE);

	auto itFind = in->mKeyItemMap.find(attr);
	if (itFind == in->mKeyItemMap.end()) {
		spdlog::debug(_T("key bind does not exist."));
		return false;
	}
	spdlog::debug(_T("key bind exists."));
	return true;
}

// ホットキーに関連付けられたものがあるかをチェック
bool CommandHotKeyManager::HasKeyBinding(const SANDSKEY_ATTR& attr)
{
	SPDLOG_DEBUG(_T("args Modifier:{0} VKCode:{1}"), attr.GetModifier(), attr.GetVKCode());

	CSingleLock sl(&in->mCS, TRUE);

	auto itFind = in->mSandSItemMap.find(attr);
	if (itFind == in->mSandSItemMap.end()) {
		spdlog::debug(_T("key bind does not exist."));
		return false;
	}
	spdlog::debug(_T("key bind exists."));
	return true;
}

bool CommandHotKeyManager::Register(
	void* owner,
	CommandHotKeyHandler* handler,
	const CommandHotKeyAttribute& key
)
{
	SPDLOG_DEBUG(_T("args name:{0} Modifier:{1} VKCode:{2} isGlobal:{3}"),
	             (LPCTSTR)handler->GetDisplayName(), key.GetModifiers(), key.GetVKCode(), key.mIsGlobal);

	if (key.IsValid() == false && key.IsValidSandS() == false) {
		return false;
	}


	CSingleLock sl(&in->mCS, TRUE);

	ASSERT(handler);

	auto dispName = handler->GetDisplayName();

	// 別の登録元によって既に登録されているキーの場合は登録を許可しない
	auto it = in->mNameKeyMap.find(dispName);
	if (it != in->mNameKeyMap.end() && it->second.mOwner != owner) {
		return false;
	}

	if (in->RegisterHotKeyAttr(handler, key) == false &&
      in->RegisterSandSKeyAttr(handler, key) == false) {
		return false;
	}

	if (handler->IsTemporaryHandler() == false) {
		in->mNameKeyMap[dispName] = NameMapValue(owner, key);
	}

	return true;
}

// 登録解除(ハンドラオブジェクトがわかっている場合利用可能)
bool CommandHotKeyManager::Unregister(CommandHotKeyHandler* handler)
{
	CSingleLock sl(&in->mCS, TRUE);

	for (auto it = in->mKeyItemMap.begin(); it != in->mKeyItemMap.end(); ++it) {
		auto& item = it->second;
		if (handler != item.mHandlerPtr.get()) {
			continue;
		}
		in->mKeyItemMap.erase(it);
		break;
	}
	for (auto it = in->mSandSItemMap.begin(); it != in->mSandSItemMap.end(); ++it) {
		auto& item = it->second;
		if (handler != item.mHandlerPtr.get()) {
			continue;
		}
		in->mSandSItemMap.erase(it);
		break;
	}
	for (auto it = in->mLocalHandlerMap.begin(); it != in->mLocalHandlerMap.end(); ++it) {
		if (handler != it->second) {
			continue;
		}
		in->mLocalHandlerMap.erase(it);
		break;
	}
	return true;
}

void CommandHotKeyManager::GetMappings(CommandHotKeyMappings& keyMap)
{
	CSingleLock sl(&in->mCS, TRUE);

	CommandHotKeyMappings tmp;
	for (auto& elem : in->mNameKeyMap) {
		const CommandHotKeyAttribute& attr = elem.second.mAttr;
		tmp.AddItem(elem.first, attr);
	}
	keyMap.Swap(tmp);
}

void CommandHotKeyManager::Clear(void* owner)
{
	SPDLOG_DEBUG(_T("start"));

	CSingleLock sl(&in->mCS, TRUE);

	for (auto it = in->mNameKeyMap.begin(); it != in->mNameKeyMap.end();) {
		if (owner != it->second.mOwner) {
			it++;
			continue;
		}

		const CommandHotKeyAttribute& cmdKeyAttr = it->second.mAttr;

		auto hotKeyAttr = cmdKeyAttr.mHotKeyAttr;
		auto it2 = in->mKeyItemMap.find(hotKeyAttr);
		if (it2 != in->mKeyItemMap.end()) {
			in->mKeyItemMap.erase(it2);
		}

		auto sandsAttr = cmdKeyAttr.mSandSKeyAttr;
		auto it3 = in->mSandSItemMap.find(sandsAttr);
		if (it3 != in->mSandSItemMap.end()) {
			in->mSandSItemMap.erase(it3);
		}

		it = in->mNameKeyMap.erase(it);

		// 変更フラグを立てておく
		in->mIsChanged = true;
	}
}

} // end of namespace core
} // end of namespace launcherapp

