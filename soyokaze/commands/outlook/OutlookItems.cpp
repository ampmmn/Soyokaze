#include "pch.h"
#include "OutlookItems.h"
#include "commands/activate_window/AutoWrap.h"
#include "utility/ScopeAttachThreadInput.h"
#include <propvarutil.h>
#include <mutex>
#include <map>
#include <set>
#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::activate_window;

namespace launcherapp {
namespace commands {
namespace outlook {

// メール(スレッド)数上限
constexpr int ITEM_LIMIT = 1024;

enum {
	STATUS_READY,   // 待機状態
	STATUS_BUSY,   //  更新中
};

struct OutlookItems::PImpl
{
	void Update();

	bool IsBusy() {
		std::lock_guard<std::mutex> lock(mMutex);
		return mStatus == STATUS_BUSY;
	}

	// 前回の取得時のタイムスタンプ
	uint64_t mLastUpdate = 0;
	int mLastItemCount = 0;
	std::map<CString, MailItem*> mMailItemMap;

	// 最新の受信日時(これ以前の日付のメールは処理済みとする)
	DWORD mLastReceivedTime = 0;

	// 生成処理の排他制御
	std::mutex mMutex;

	int mStatus = STATUS_BUSY;
};

static DWORD GetReceivedTime(
	DispWrapper& item
)
{
	union timeunion {
		struct {
			WORD receivedDate;
			WORD receivedTime;
		} st;
		DWORD dwTime;
	} t;

	VARIANT result = {};
	item.GetPropertyVariant(L"ReceivedTime", result);
	VariantToDosDateTime(result, &t.st.receivedDate, &t.st.receivedTime);

	return t.dwTime;
}

void OutlookItems::PImpl::Update()
{
	{
		std::lock_guard<std::mutex> lock(mMutex);
		mStatus = STATUS_BUSY;
	}

	std::thread th([&]() {
		// OutlookのCLSIDを得る
		CLSID clsid;
		HRESULT hr = CLSIDFromProgID(L"Outlook.Application", &clsid);

		if (FAILED(hr)) {
			// 取得できなかった(インストールされていないとか)
			std::lock_guard<std::mutex> lock(mMutex);
			mLastUpdate = GetTickCount64();
			mStatus = STATUS_READY;
			return ;
		}

		// 既存のOutlook.Applicationインスタンスを取得する
		CComPtr<IUnknown> unkPtr;
		hr = GetActiveObject(clsid, NULL, &unkPtr);
		if(FAILED(hr)) {
			// 起動してない
			std::lock_guard<std::mutex> lock(mMutex);

			for (auto& elem : mMailItemMap) {
				auto& mailItemPtr = elem.second;
				mailItemPtr->Release();
			}
			mMailItemMap.clear();

			mLastUpdate = GetTickCount64();
			mStatus = STATUS_READY;
			return ;
		}

		DispWrapper outlookApp;
		unkPtr->QueryInterface(&outlookApp);

		VARIANT result;

		// Get MAPI object
		DispWrapper mapi;
		outlookApp.CallObjectMethod(L"GetNameSpace", L"MAPI", mapi);

		// 受信トレイ(Inbox)のフォルダを取得
		DispWrapper inboxFolder;
		mapi.CallObjectMethod(L"GetDefaultFolder", (int32_t)6, inboxFolder);  // 6 means olFolderInbox

		// フォルダのItemsを取得
		DispWrapper items;
		inboxFolder.GetPropertyObject(L"Items", items);

		// 受信日時降順でソート
		{
			CComBSTR argVal(L"[ReceivedTime]");
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_BSTR;
			arg1.bstrVal = argVal;

			VARIANT arg2;
			VariantInit(&arg2);
			arg2.vt = VT_BOOL;
			arg2.boolVal = VARIANT_TRUE;

			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, items, L"Sort", 2, &arg1, &arg2);
		}

		// メール数を取得
		int itemCount = items.GetPropertyInt(L"Count");

		if (itemCount == mLastItemCount) {
			// 前回とメール件数が同じだったら変化がないものとみなす
			// (操作次第で、新着メールの取りこぼしの可能性はあるが..)
			mLastUpdate = GetTickCount64();
			mStatus = STATUS_READY;
			return ;
		}
		mLastItemCount = itemCount;

		std::set<CString> conversationSet;

		// メール一つひとつづつみて、スレッド一覧を生成する
		std::vector<MailItem*> tmpList;

		for (int i = 1; i <= itemCount; ++i) {

			Sleep(0);

			// 上限を超えたら処理を止める
			if (tmpList.size() + mMailItemMap.size() >= ITEM_LIMIT) {
				break;
			}

			// 要素(MailItem)を取得
			DispWrapper item;
			items.GetPropertyObject(L"Item", (int32_t)i, item);

			// 受信日時を見る
			// 前回処理時に、最新だったメールの受信日時に達したら処理終了
			DWORD dwReceived = GetReceivedTime(item);
			if (dwReceived <= mLastReceivedTime) {
				break;
			}

			// ConversationIDを得る
			CString conversationID = item.GetPropertyString(L"ConversationID");

			auto it = conversationSet.find(conversationID);
			if (it != conversationSet.end()) {
				// 同じスレッドのメールを既に取得済
				continue;
			}
			conversationSet.insert(conversationID);

			// Subjectを得る
			CString subject = item.GetPropertyString(L"Subject");
			tmpList.push_back(new MailItem(conversationID, subject));
		}

		// 直近のメール受信日時を覚えておく
		DispWrapper item;
		items.GetPropertyObject(L"Item", (int32_t)1, item);
		mLastReceivedTime = GetReceivedTime(item);

		std::lock_guard<std::mutex> lock(mMutex);
		for (auto& newItem : tmpList) {

			auto& key = newItem->GetConversationID();

			auto it = mMailItemMap.find(key);
			if (it != mMailItemMap.end()) {
				(it->second)->Release();
			}

			mMailItemMap[key] = newItem;
		}

		mLastUpdate = GetTickCount64();
		mStatus = STATUS_READY;
	});
	th.detach();
}

OutlookItems::OutlookItems() : in(std::make_unique<PImpl>())
{
	HRESULT hr = CoInitialize(NULL);
	if (FAILED(hr)) {
		SPDLOG_ERROR(_T("Failed to CoInitialize!"));
	}

	in->mLastUpdate = 0;
	in->mLastItemCount = 0;
	in->mStatus = STATUS_READY;
}

OutlookItems::~OutlookItems()
{
	std::lock_guard<std::mutex> lock(in->mMutex);
	for (auto& elem : in->mMailItemMap) {
		elem.second->Release();
	}
	in->mMailItemMap.clear();

	CoUninitialize();
}

// この時間以内に再実行されたら、前回の結果を再利用する
constexpr int INTERVAL_REUSE = 5000;

bool OutlookItems::GetInboxMailItems(std::vector<MailItem*>& mailItems)
{
	ASSERT(mailItems.empty());

	// 前回取得時から一定時間経過していない場合は前回の結果を再利用する
	uint64_t elapsed = GetTickCount64() - in->mLastUpdate;

	if (elapsed >= INTERVAL_REUSE) {
		if (in->IsBusy() == false) {
			// メール一覧を再取得する
			in->Update();
		}
	}

	std::lock_guard<std::mutex> lock(in->mMutex);

	mailItems.reserve(in->mMailItemMap.size());
	for (auto& elem : in->mMailItemMap) {

		auto& mailItemPtr = elem.second;

		mailItems.push_back(mailItemPtr);
		mailItemPtr->AddRef();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


struct MailItem::PImpl
{
	// 会話ID
	CString mConversationID;
	// 件名
	CString mSubject;
	// 参照カウント
	uint32_t mRefCount = 1;
	
};

/**
 	コンストラクタ
 	@param[in] conversationID 会話ID
 	@param[in] subject        件名
*/
 MailItem::MailItem(
	const CString& conversationID,
	const CString& subject
) : 
	in(std::make_unique<PImpl>())
{
	in->mConversationID = conversationID;
	in->mSubject = subject;
}

MailItem::~MailItem()
{
}

const CString& MailItem::GetConversationID()
{
	return in->mConversationID;
}

const CString& MailItem::GetSubject()
{
	return in->mSubject;
}

/**
 	オブジェクトに紐づけられたワークシートを有効にする
 	@return 処理の成否
*/
BOOL MailItem::Activate(bool isShowMaximize)
{
	UNREFERENCED_PARAMETER(isShowMaximize);

	CLSID clsid;
	HRESULT hr = CLSIDFromProgID(L"Outlook.Application", &clsid);

	if (FAILED(hr)) {
		// 初期化できなかった(たぶん起こらない。このオブジェクト作れてる時点でOutlookは入ってるはずなので)
		return FALSE;
	}

	// 既存のOutlook.Applicationインスタンスを取得する
	CComPtr<IUnknown> unkPtr;
	hr = GetActiveObject(clsid, NULL, &unkPtr);
	if(FAILED(hr)) {
		// インスタンス生成後にアプリケーションが終了されたとか?
		return FALSE;
	}

	DispWrapper outlookApp;
	unkPtr->QueryInterface(&outlookApp);

	VARIANT result;

	// Outlook.ApplicationからたどってMailItemを得る

	DispWrapper mapi;
	outlookApp.CallObjectMethod(L"GetNameSpace", L"MAPI", mapi);

	// 受信トレイ(Inbox)のフォルダを取得
	DispWrapper inboxFolder;
	mapi.CallObjectMethod(L"GetDefaultFolder", (int32_t)6, inboxFolder);   // 6 means olFolderInbox

	// フォルダのItemsを取得
	DispWrapper items;
	inboxFolder.GetPropertyObject(L"Items", items);

	// 受信日時降順でソート
	{
		CComBSTR argVal(L"[ReceivedTime]");
		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		VARIANT arg2;
		VariantInit(&arg2);
		arg2.vt = VT_BOOL;
		arg2.boolVal = VARIANT_TRUE;

		VariantInit(&result);
		AutoWrap(DISPATCH_METHOD, &result, items, L"Sort", 2, &arg1, &arg2);
	}

	// メール数を取得
	int itemCount = items.GetPropertyInt(L"Count");

	// メール一つひとつづつみて、ConversationIDが一致するメール要素を探す
	for (int i = 1; i <= itemCount; ++i) {

		Sleep(0);

		// 要素(MailItem)を取得
		DispWrapper item;
		items.GetPropertyObject(L"Item", (int32_t)i, item);


		// ConversationIDを得る
		CString conversationID = item.GetPropertyString(L"ConversationID");
		if (conversationID != in->mConversationID) {
			continue;
		}

		// GetConversationでConversation取得
		DispWrapper conversation;
		item.CallObjectMethod(L"GetConversation", conversation);

		// Conversationオブジェクトからスレッド内メール項目の一覧を取得
		DispWrapper itemCollection;
		conversation.CallObjectMethod(L"GetChildren", item, itemCollection);

		// スレッド内メール数を取得
		int threadItemCount = itemCollection.GetPropertyInt(L"Count");

		DispWrapper latestItem;
		if (threadItemCount == 0) {
			latestItem = item;
		}

		// 個々のメールの受信日時を見て、直近のものを選択
		WORD lastReceivedDate = 0;
		WORD lastReceivedTime = 0;
		for (int j = 1; j <= threadItemCount; ++j) {
			DispWrapper childItem;
			itemCollection.CallObjectMethod(L"Item", (int32_t)j, childItem);

			// 受信日時が最も最近のもの
			WORD receivedDate;
			WORD receivedTime;
			childItem.GetPropertyVariant(L"ReceivedTime", result);
			VariantToDosDateTime(result, &receivedDate, &receivedTime);

			if (receivedDate < lastReceivedDate) {
				continue;
			}
			if (receivedDate == lastReceivedDate && receivedTime < lastReceivedTime) {
				continue;
			}

			lastReceivedTime = receivedTime;
			lastReceivedDate = receivedDate;

			latestItem = childItem;
		}

		if (!latestItem) {
			break;
		}

		// スレッドの最新のメールをポップアップで表示
		latestItem.CallVoidMethod(L"Display");
		break;
	}
	return TRUE;
}


uint32_t MailItem::AddRef()
{
	return ++in->mRefCount;
}

uint32_t MailItem::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

} // end of namespace outlook
} // end of namespace commands
} // end of namespace launcherapp

