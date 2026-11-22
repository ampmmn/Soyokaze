#include "pch.h"
#include "SearchViewerProxy.h"
#include "commands/websearch/SearchViewerWindow.h"
#include "features/main/controller/MainWindowController.h"
#include <map>
#include <functional>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace websearch {

namespace {

struct CommandNameLess
{
	/** コマンド名を大文字小文字を区別せず比較する */
	bool operator()(const std::wstring& lhs, const std::wstring& rhs) const
	{
		return CompareStringOrdinal(lhs.c_str(), static_cast<int>(lhs.size()), rhs.c_str(), static_cast<int>(rhs.size()), TRUE) == CSTR_LESS_THAN;
	}
};

/** CStringをマップ用の文字列へ変換する */
std::wstring ToWideString(const CString& value)
{
	return std::wstring(value.GetString(), value.GetLength());
}

}

struct SearchViewerProxy::PImpl
{
	/** UIスレッドでコールバックを実行する */
	bool InvokeOnUiThread(std::function<void()> callback)
	{
		if (mUiThreadId == 0) {
			return false;
		}
		if (GetCurrentThreadId() == mUiThreadId) {
			callback();
			return true;
		}

		auto controller = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		controller->RequestCallback([](LPARAM param) -> LRESULT {
			auto callback = reinterpret_cast<const std::function<void()>*>(param);
			(*callback)();
			return 0;
		}, &callback);
		return true;
	}

	struct Registration
	{
		RegistrationToken mToken{0};
		CString mTitle;
	};

	/** コマンドの検索ウインドウ設定を登録する */
	RegistrationToken RegisterCommand(const CString& commandName, const CString& title)
	{
		if (mIsClosing || commandName.IsEmpty()) {
			return 0;
		}

		CString actualTitle = SearchViewerProxy::CreateWindowTitle(commandName, title);

		const auto key = ToWideString(commandName);
		const RegistrationToken token = ++mNextToken;
		mRegistrations[key] = Registration{ token, actualTitle };

		auto window = mWindows.find(key);
		if (window != mWindows.end()) {
			window->second->SetTitle(actualTitle);
		}
		return token;
	}

	/** 登録トークンが一致するときだけコマンド設定を解除する */
	void UnregisterCommand(const CString& commandName, RegistrationToken token)
	{
		const auto it = mRegistrations.find(ToWideString(commandName));
		if (it != mRegistrations.end() && it->second.mToken == token) {
			mRegistrations.erase(it);
		}
	}

	/** 登録済みコマンドの検索ウインドウを表示または再利用する */
	bool Open(const CString& commandName, const CString& url)
	{
		if (mIsClosing) {
			return false;
		}

		const auto key = ToWideString(commandName);
		const auto registration = mRegistrations.find(key);
		if (registration == mRegistrations.end()) {
			return false;
		}

		auto window = mWindows.find(key);
		if (window != mWindows.end()) {
			return window->second->Open(registration->second.mTitle, url);
		}

		auto newWindow = std::make_unique<SearchViewerWindow>(commandName);
		if (newWindow->Open(registration->second.mTitle, url) == false) {
			return false;
		}
		mWindows.emplace(key, std::move(newWindow));
		return true;
	}

	/** 登録済みの全ウインドウを閉じてProxyを終了状態にする */
	void CloseAll()
	{
		mIsClosing = true;
		for (auto& item : mWindows) {
			item.second->Close();
		}
		mWindows.clear();
		mRegistrations.clear();
	}

	RegistrationToken mNextToken{0};
	bool mIsClosing{false};
	DWORD mUiThreadId{0};
	std::map<std::wstring, Registration, CommandNameLess> mRegistrations;
	std::map<std::wstring, std::unique_ptr<SearchViewerWindow>, CommandNameLess> mWindows;
};

SearchViewerProxy::SearchViewerProxy() : in(std::make_unique<PImpl>())
{
}

SearchViewerProxy::~SearchViewerProxy()
{
	CloseAll();
}

SearchViewerProxy* SearchViewerProxy::GetInstance()
{
	// アプリ終了時のCOM解放後にコマンドが破棄されても安全なよう、明示終了まで保持する
	static SearchViewerProxy* instance = new SearchViewerProxy();
	return instance;
}

void SearchViewerProxy::InitializeOnUiThread()
{
	const auto threadId = GetCurrentThreadId();
	ASSERT(in->mUiThreadId == 0 || in->mUiThreadId == threadId);
	in->mUiThreadId = threadId;
}

CString SearchViewerProxy::CreateWindowTitle(const CString& commandName, const CString& title)
{
	if (title.IsEmpty() == FALSE) {
		return title;
	}
	CString result;
	result.Format(_T("%sの検索結果"), (LPCTSTR)commandName);
	return result;
}

SearchViewerProxy::RegistrationToken SearchViewerProxy::RegisterCommand(const CString& commandName, const CString& title)
{
	RegistrationToken token{0};
	in->InvokeOnUiThread([&]() {
		token = in->RegisterCommand(commandName, title);
	});
	return token;
}

void SearchViewerProxy::UnregisterCommand(const CString& commandName, RegistrationToken token)
{
	in->InvokeOnUiThread([&]() {
		in->UnregisterCommand(commandName, token);
	});
}

bool SearchViewerProxy::Open(const CString& commandName, const CString& url)
{
	bool result{false};
	in->InvokeOnUiThread([&]() {
		result = in->Open(commandName, url);
	});
	return result;
}

void SearchViewerProxy::CloseAll()
{
	in->InvokeOnUiThread([&]() {
		in->CloseAll();
	});
}

}
}
}
