#include "pch.h"
#include "CommandQueryReceiver.h"
#include "commands/core/CommandParameter.h"
#include "commands/core/CommandQueryRequest.h"
#include "commands/core/CommandRepository.h"

namespace launcherapp { namespace remote {

using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;
using CommandRepository = launcherapp::core::CommandRepository;

constexpr UINT WM_APP_QUERYDONE = WM_APP+1;

// 検索結果を受け取る入れ物としての構造体
struct QueryReceiver::QUERY_RESULT
{
	// 検索中か?
	bool mIsQueryDoing{false};
	// 検索結果
	std::vector<launcherapp::core::Command*> mCommands;
};

QueryReceiver::QueryReceiver()
{
}

QueryReceiver::~QueryReceiver()
{
	// ウインドウがある場合は破棄する
	if (IsWindow(mHwnd)) {
		DestroyWindow(mHwnd);
		mHwnd = nullptr;
	}
}

void QueryReceiver::QuerySync(
	const std::wstring& query,
	std::vector<launcherapp::core::Command*>& commands
)
{
	// 初回はウインドウ作成
	if (IsWindow(mHwnd) == FALSE) {
		CreateReceiverWindow();
	}

	// リクエストを出す
	QUERY_RESULT result{true};
	SetWindowLongPtr(mHwnd, GWLP_USERDATA, (LONG_PTR)&result);

	auto commandParam = CommandParameterBuilder::Create(query.c_str());
	launcherapp::commands::core::CommandQueryRequest req(commandParam, mHwnd, WM_APP_QUERYDONE);
	CommandRepository::GetInstance()->Query(req);

	// 結果が返ってくるまで待つ
	while(result.mIsQueryDoing) {
		MSG msg;
		GetMessage(&msg, 0, NULL, NULL);
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// 結果を取得する。
	// (参照カウントは呼び出し元側で下げること)
	commands = result.mCommands;
}

// 結果を受け取るためのウインドウを作成する
bool QueryReceiver::CreateReceiverWindow()
{
	// クリップボードイベントを受信するためのウィンドウを作成する
	HINSTANCE hInst = GetModuleHandle(nullptr);
	HWND hwnd = CreateWindowEx(0, _T("STATIC"), _T("QueryReceiver"), 0, 
	                           0, 0, 0, 0, nullptr, nullptr, hInst, nullptr);
	if (hwnd == nullptr) {
		spdlog::error("[remote]failed to receiver window.");
		return false;
	}

	// コールバック関数を登録する
	SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)OnWindowProc);

	mHwnd = hwnd;
	return true;
}

LRESULT CALLBACK
QueryReceiver::OnWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	// 取り扱うメッセージ以外の場合はデフォルト処理をする
	if (msg != WM_APP_QUERYDONE) {
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	// ユーザ定義パラメータ(QUERY_RESULT)を取得
	auto result = (QUERY_RESULT*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	if (result == nullptr) {
		spdlog::warn("[remote]GetWindowLongPtr returned null.");
		return 0;
	}
	// (念のため)次回の呼び出しで再度入らないよう、ユーザ定義パラメータをリセットする
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)nullptr);

	// 検索中フラグの状態を変更し、検索結果を受け取る
	result->mIsQueryDoing = false;

	auto commands = (std::vector<launcherapp::core::Command*>*)lparam;
	if (commands == nullptr) {
		return 0;
	}

	if (commands != nullptr) {
		result->mCommands.swap(*commands);
		commands->clear();
		delete commands;
	}
	else {
		result->mCommands.clear();
	}
	return 0;
}

}} // end of namespace launcherapp::remote


