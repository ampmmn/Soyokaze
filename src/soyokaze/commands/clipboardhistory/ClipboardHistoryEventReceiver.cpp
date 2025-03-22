#include "pch.h"
#include "ClipboardHistoryEventReceiver.h"
#include <set>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace clipboardhistory {

/**
 * @brief ClipboardHistoryEventReceiver の内部実装クラス
 */
struct ClipboardHistoryEventReceiver::PImpl
{
	/**
	 * @brief クリップボード更新時の処理
	 * @return 常に 0 を返す
	 */
	int OnUpdateClipboard();

	HWND mReceiverWindow = nullptr; ///< クリップボードイベントを受信するウィンドウのハンドル
	std::set<ClipboardHistoryEventListener*> mListeners; ///< クリップボードイベントリスナーのセット
	int mInterval = 500; ///< クリップボード更新のインターバル
	std::unique_ptr<tregex> mExcludePattern; ///< 除外パターンの正規表現
	uint64_t mLastUpdated = 0; ///< 最後に更新された時刻
	bool mIsUpdating = false; ///< 更新中かどうかのフラグ
};

/**
 * @brief クリップボード更新時の処理
 * @return 常に 0 を返す
 */
int ClipboardHistoryEventReceiver::PImpl::OnUpdateClipboard()
{
	if (mIsUpdating) {
		// 再入防止
		spdlog::debug("OnUpdateClipboard blocked.");
		return 0;
	}

	// 関数をぬけるときにフラグを戻す
	struct scope_flag {
		scope_flag(bool& f) : mFlag(f) { f = true; }
		~scope_flag() { mFlag = false; }
		bool& mFlag;
	} flag(mIsUpdating);

	// 前回の通知から間が空いていない場合は無視
	if (GetTickCount64() - mLastUpdated < mInterval) {
		return 0;
	}

	// クリップボードの内容を取得する
	tstring newData;

	// クリップボードを開く
	if (!OpenClipboard(nullptr)) {
		spdlog::error("Failed to OpenClipboard errCode:{}", GetLastError());
		return 0;
	}

	// テキストデータを取得
	bool isNotTextData = false;
	HANDLE hData = GetClipboardData(CF_UNICODETEXT);
	if (hData) {
		wchar_t* pszText = static_cast<wchar_t*>(GlobalLock(hData));
		if (pszText) {
			newData = pszText;
			GlobalUnlock(hData);
		}
		else {
			spdlog::error("Failed to OpenClipboard errCode:{}", GetLastError());
		}
	} else {
		isNotTextData = true;
	}

	// クリップボードを閉じる
	CloseClipboard();

	if (isNotTextData) {
		// テキストでない場合は更新イベントを発行しない。
		// このときも更新されたとみなし、前回の通知時刻を覚えておく
		mLastUpdated = GetTickCount64();
		return 0;
	}

	// 無視するパターンに合致する場合は通知しない
	if (mExcludePattern.get() && std::regex_match(newData, *mExcludePattern.get())) {
		return 0;
	}

	// リスナーへ通知
	for (auto& listener : mListeners) {
		listener->UpdateClipboard(newData.c_str());
	}

	// 前回の通知時刻を覚えておく
	mLastUpdated = GetTickCount64();
	return 0;
}

/**
 * @brief コンストラクタ
 */
ClipboardHistoryEventReceiver::ClipboardHistoryEventReceiver() : in(new PImpl)
{
}

/**
 * @brief デストラクタ
 */
ClipboardHistoryEventReceiver::~ClipboardHistoryEventReceiver()
{
	if (in->mReceiverWindow) {
		::DestroyWindow(in->mReceiverWindow);
		in->mReceiverWindow = nullptr;
	}
}

/**
 * @brief 初期化処理
 * @return 初期化に成功した場合は true、失敗した場合は false
 */
bool ClipboardHistoryEventReceiver::Initialize()
{
	if (in->mReceiverWindow == nullptr) {
		// クリップボードイベントを受信するためのウィンドウを作成する
		HINSTANCE hInst = AfxGetInstanceHandle();
		HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("LncrClipboardReceiver"), 0, 
		                           0, 0, 0, 0, nullptr, nullptr, hInst, nullptr);
		ASSERT(hwnd);

		SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

		in->mReceiverWindow = hwnd;
	}
	return true;
}

/**
 * @brief クリップボードイベントの受信を開始する
 * @param interval 更新のインターバル
 * @param excludePattern 除外パターンの正規表現
 * @return 成功した場合は true、失敗した場合は false
 */
bool ClipboardHistoryEventReceiver::Activate(int interval, const CString& excludePattern)
{
	if (AddClipboardFormatListener(in->mReceiverWindow) == FALSE) {
		spdlog::warn("Failed to AddClipboardFormatListener. errCode:{:x}", GetLastError());
	}

	in->mInterval = interval;
	try {
		if (excludePattern.IsEmpty() == FALSE) {
			in->mExcludePattern.reset(new tregex(tstring((LPCTSTR)excludePattern)));
		}
		else {
			in->mExcludePattern.reset();
		}
		return true;
	}
	catch(std::regex_error& e) {
		spdlog::error(_T("Failed to initialize regex. pattern:{0} cause:{1}"),
		              (LPCTSTR)excludePattern, (LPCTSTR)e.what());
		return false;
	}
}

/**
 * @brief クリップボードイベントの受信を停止する
 */
void ClipboardHistoryEventReceiver::Deactivate()
{
	if (in->mReceiverWindow) {
		RemoveClipboardFormatListener(in->mReceiverWindow);
	}
	in->mListeners.clear();
}

/**
 * @brief クリップボードイベントリスナーを追加する
 * @param listener 追加するリスナー
 */
void ClipboardHistoryEventReceiver::AddListener(ClipboardHistoryEventListener* listener)
{
	in->mListeners.insert(listener);
}

/**
 * @brief ウィンドウプロシージャ
 * @param hwnd ウィンドウハンドル
 * @param msg メッセージ
 * @param wparam WPARAM
 * @param lparam LPARAM
 * @return メッセージの処理結果
 */
LRESULT CALLBACK ClipboardHistoryEventReceiver::OnWindowProc(
	HWND hwnd,
	UINT msg,
	WPARAM wparam,
	LPARAM lparam
)
{
	if (msg == WM_CLIPBOARDUPDATE) {
		// クリップボード更新通知
		auto thisPtr = (ClipboardHistoryEventReceiver*)(size_t)GetWindowLongPtr(hwnd, GWLP_USERDATA);
		return thisPtr->in->OnUpdateClipboard();
	}

	return DefWindowProc(hwnd, msg, wparam ,lparam);
}

} // end of namespace clipboardhistory
} // end of namespace commands
} // end of namespace launcherapp



