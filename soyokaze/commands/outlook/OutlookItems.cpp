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

using namespace soyokaze::commands::activate_window;

namespace soyokaze {
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
	DWORD mLastUpdate;
	int mLastItemCount;
	std::map<CString, MailItem*> mMailItemMap;

	// 最新の受信日時(これ以前の日付のメールは処理済みとする)
	DWORD mLastReceivedTime = 0;

	// 生成処理の排他制御
	std::mutex mMutex;

	int mStatus;
};

static bool GetMailItem(
	CComPtr<IDispatch>& items,
	int index,
	CComPtr<IDispatch>& item
)
{
	VARIANT arg1;
	VariantInit(&arg1);
	arg1.vt = VT_INT;
	arg1.intVal = index;

	VARIANT result;
	VariantInit(&result);

	HRESULT hr = AutoWrap(DISPATCH_PROPERTYGET, &result, items, L"Item", 1, &arg1);
	item = result.pdispVal;

	return SUCCEEDED(hr);
}

static DWORD GetReceivedTime(
	CComPtr<IDispatch>& item
)
{
	union timeunion {
		struct {
			WORD receivedDate;
			WORD receivedTime;
		} st;
		DWORD dwTime;
	} t;

	VARIANT result;
	VariantInit(&result);
	AutoWrap(DISPATCH_PROPERTYGET, &result, item, L"ReceivedTime", 0);

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
			mLastUpdate = GetTickCount();
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

			mLastUpdate = GetTickCount();
			mStatus = STATUS_READY;
			return ;
		}

		CComPtr<IDispatch> outlookApp;
		unkPtr->QueryInterface(&outlookApp);

		VARIANT result;

		// Get MAPI object
		CComPtr<IDispatch> mapi;
		{
			CComBSTR argVal(L"MAPI");
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_BSTR;
			arg1.bstrVal = argVal;

			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, outlookApp, L"GetNameSpace", 1, &arg1);
			mapi = result.pdispVal;
		}

		// 受信トレイ(Inbox)のフォルダを取得
		CComPtr<IDispatch> inboxFolder;
		{
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_INT;
			arg1.intVal = 6;   // 6 means olFolderInbox

			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, mapi, L"GetDefaultFolder", 1, &arg1);
			inboxFolder = result.pdispVal;
		}

		// フォルダのItemsを取得
		CComPtr<IDispatch> items;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, inboxFolder, L"Items", 0);
			items = result.pdispVal;
		}

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
		int itemCount = 0;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, items, L"Count", 0);
			itemCount = result.intVal;
		}

		if (itemCount == mLastItemCount) {
			// 前回とメール件数が同じだったら変化がないものとみなす
			// (操作次第で、新着メールの取りこぼしの可能性はあるが..)
			mLastUpdate = GetTickCount();
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
			CComPtr<IDispatch> item;
			GetMailItem(items, i, item);

			// 受信日時を見る
			// 前回処理時に、最新だったメールの受信日時に達したら処理終了
			DWORD dwReceived = GetReceivedTime(item);
			if (dwReceived <= mLastReceivedTime) {
				break;
			}

			CComBSTR strVal;
			// ConversationIDを得る
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, item, L"ConversationID", 0);

				strVal = result.bstrVal;
			}
			CString conversationID = strVal;

			auto it = conversationSet.find(conversationID);
			if (it != conversationSet.end()) {
				// 同じスレッドのメールを既に取得済
				continue;
			}
			conversationSet.insert(conversationID);

			// Subjectを得る
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, item, L"Subject", 0);

				strVal = result.bstrVal;
			}

			CString subject = strVal;
			tmpList.push_back(new MailItem(conversationID, subject));
		}

		// 直近のメール受信日時を覚えておく
		CComPtr<IDispatch> item;
		GetMailItem(items, 1, item);
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

		mLastUpdate = GetTickCount();
		mStatus = STATUS_READY;
	});
	th.detach();
}

OutlookItems::OutlookItems() : in(std::make_unique<PImpl>())
{
	CoInitialize(NULL);

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
	DWORD elapsed = GetTickCount() - in->mLastUpdate;

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
	uint32_t mRefCount;
	
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
	in->mRefCount = 1;

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

	CComPtr<IDispatch> outlookApp;
	unkPtr->QueryInterface(&outlookApp);

	VARIANT result;

	// Outlook.ApplicationからたどってMailItemを得る

	CComPtr<IDispatch> mapi;
	{
		CComBSTR argVal(L"MAPI");
		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_BSTR;
		arg1.bstrVal = argVal;

		VariantInit(&result);
		AutoWrap(DISPATCH_METHOD, &result, outlookApp, L"GetNameSpace", 1, &arg1);
		mapi = result.pdispVal;
	}

	// 受信トレイ(Inbox)のフォルダを取得
	CComPtr<IDispatch> inboxFolder;
	{
		VARIANT arg1;
		VariantInit(&arg1);
		arg1.vt = VT_INT;
		arg1.intVal = 6;   // 6 means olFolderInbox

		VariantInit(&result);
		AutoWrap(DISPATCH_METHOD, &result, mapi, L"GetDefaultFolder", 1, &arg1);
		inboxFolder = result.pdispVal;
	}

	// フォルダのItemsを取得
	CComPtr<IDispatch> items;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, inboxFolder, L"Items", 0);
		items = result.pdispVal;
	}

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
	int itemCount = 0;
	{
		VariantInit(&result);
		AutoWrap(DISPATCH_PROPERTYGET, &result, items, L"Count", 0);
		itemCount = result.intVal;
	}

	// メール一つひとつづつみて、ConversationIDが一致するメール要素を探す
	for (int i = 1; i <= itemCount; ++i) {

		Sleep(0);

		// 要素(MailItem)を取得
		CComPtr<IDispatch> item;
		GetMailItem(items, i, item);

		CComBSTR strVal;

		// ConversationIDを得る
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, item, L"ConversationID", 0);

			strVal = result.bstrVal;
		}
		CString conversationID = strVal;

		if (conversationID != in->mConversationID) {
			continue;
		}

		// GetConversationでConversation取得
		CComPtr<IDispatch> conversation;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, item, L"GetConversation", 0);
			conversation = result.pdispVal;
		}

		// Conversationオブジェクトからスレッド内メール項目の一覧を取得
		CComPtr<IDispatch> itemCollection;
		{
			VARIANT arg1;
			VariantInit(&arg1);
			arg1.vt = VT_DISPATCH;
			arg1.pdispVal = item;

			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, conversation, L"GetChildren", 1, &arg1);
			itemCollection = result.pdispVal;
		}

		// スレッド内メール数を取得
		int threadItemCount;
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_PROPERTYGET, &result, itemCollection, L"Count", 0);
			threadItemCount = result.intVal;
		}

		CComPtr<IDispatch> latestItem;
		if (threadItemCount == 0) {
			latestItem = item;
		}

		// 個々のメールの受信日時を見て、直近のものを選択
		WORD lastReceivedDate = 0;
		WORD lastReceivedTime = 0;
		for (int j = 1; j <= threadItemCount; ++j) {
			CComPtr<IDispatch> childItem;
			{
				VARIANT arg1;
				VariantInit(&arg1);
				arg1.vt = VT_INT;
				arg1.intVal = j;

				VariantInit(&result);
				AutoWrap(DISPATCH_METHOD, &result, itemCollection, L"Item", 1, &arg1);
				childItem = result.pdispVal;
			}

			// 受信日時が最も最近のもの
			WORD receivedDate;
			WORD receivedTime;
			{
				VariantInit(&result);
				AutoWrap(DISPATCH_PROPERTYGET, &result, childItem, L"ReceivedTime", 0);
				VariantToDosDateTime(result, &receivedDate, &receivedTime);
			}

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
		{
			VariantInit(&result);
			AutoWrap(DISPATCH_METHOD, &result, latestItem, L"Display", 0);
		}
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
} // end of namespace soyokaze

