// fileproperty.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "fileproperty.h"

#include <shellapi.h>
#include <shlobj.h>

#include <set>
#include <string>
#include <vector>

#pragma comment(lib, "Shell32.lib")

namespace {

constexpr UINT_PTR PROPERTY_WINDOW_TIMER_ID = 1;
constexpr UINT PROPERTY_WINDOW_TIMER_INTERVAL = 100;
constexpr UINT PROPERTY_WINDOW_WAIT_TIMEOUT = 5000;
constexpr wchar_t PROPERTY_WINDOW_WAITER_CLASS_NAME[] = L"SoyokazeFilePropertyWaiter";

struct PropertyWindowWaitContext
{
	DWORD processId;
	HWND waiterWindow;
	bool currentWindowFound;
	bool propertyWindowFound;
	UINT elapsed;
};

/**
  指定されたプロセスに属するトップレベルウインドウを列挙する
  	@param[in] hwnd 列挙中のウインドウ
  	@param[in] lParam 待機状態
  	@return TRUE:列挙継続 FALSE:列挙終了
*/
BOOL CALLBACK FindPropertyWindow(HWND hwnd, LPARAM lParam)
{
	PropertyWindowWaitContext& context = *reinterpret_cast<PropertyWindowWaitContext*>(lParam);
	DWORD processId = 0;
	GetWindowThreadProcessId(hwnd, &processId);
	if (processId == context.processId) {
		context.currentWindowFound = true;
		return FALSE;
	}
	return TRUE;
}

/**
  プロパティウインドウの待機用メッセージプロシージャ
  	@param[in] hwnd ウインドウハンドル
  	@param[in] message メッセージ
  	@param[in] wParam メッセージパラメータ
  	@param[in] lParam メッセージパラメータ
  	@return メッセージ処理結果
*/
LRESULT CALLBACK PropertyWindowWaiterProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_NCCREATE) {
		const CREATESTRUCTW* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lParam);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
	}

	if (message == WM_TIMER && wParam == PROPERTY_WINDOW_TIMER_ID) {
		PropertyWindowWaitContext* context = reinterpret_cast<PropertyWindowWaitContext*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (context != nullptr) {
			context->currentWindowFound = false;
			EnumWindows(FindPropertyWindow, reinterpret_cast<LPARAM>(context));
			if (context->currentWindowFound) {
				context->propertyWindowFound = true;
				context->elapsed = 0;
			}
			else if (context->propertyWindowFound || context->elapsed >= PROPERTY_WINDOW_WAIT_TIMEOUT) {
				PostQuitMessage(0);
			}
			else {
				context->elapsed += PROPERTY_WINDOW_TIMER_INTERVAL;
			}
		}
	}

	return DefWindowProcW(hwnd, message, wParam, lParam);
}

/**
  プロパティウインドウ待機用のウインドウクラスを登録する
  	@return true:成功 false:失敗
*/
bool RegisterPropertyWindowWaiterClass()
{
	WNDCLASSEXW windowClass{};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.lpfnWndProc = PropertyWindowWaiterProc;
	windowClass.hInstance = GetModuleHandleW(nullptr);
	windowClass.lpszClassName = PROPERTY_WINDOW_WAITER_CLASS_NAME;
	if (RegisterClassExW(&windowClass) != 0) {
		return true;
	}
	return GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

/**
  プロパティウインドウを表示し、閉じるまでメッセージを処理する
  	@param[in] paths 表示対象のパス
  	@return true:待機成功 false:待機失敗
*/
bool WaitForPropertyWindow(const std::vector<std::wstring>& paths)
{
	if (RegisterPropertyWindowWaiterClass() == false) {
		return false;
	}

	PropertyWindowWaitContext context{ GetCurrentProcessId(), nullptr, false, false, 0 };
	context.waiterWindow = CreateWindowExW(
		0,
		PROPERTY_WINDOW_WAITER_CLASS_NAME,
		nullptr,
		0,
		0,
		0,
		0,
		0,
		HWND_MESSAGE,
		nullptr,
		GetModuleHandleW(nullptr),
		&context);
	if (context.waiterWindow == nullptr) {
		return false;
	}

	if (SetTimer(context.waiterWindow, PROPERTY_WINDOW_TIMER_ID, PROPERTY_WINDOW_TIMER_INTERVAL, nullptr) == 0) {
		DestroyWindow(context.waiterWindow);
		return false;
	}

	bool propertyWindowRequested = false;
	for (const std::wstring& path : paths) {
		if (SHObjectProperties(nullptr, SHOP_FILEPATH, path.c_str(), nullptr) != FALSE) {
			propertyWindowRequested = true;
		}
	}
	if (propertyWindowRequested == false) {
		KillTimer(context.waiterWindow, PROPERTY_WINDOW_TIMER_ID);
		DestroyWindow(context.waiterWindow);
		return false;
	}

	MSG message{};
	bool result = true;
	for (;;) {
		const BOOL messageResult = GetMessageW(&message, nullptr, 0, 0);
		if (messageResult <= 0) {
			result = messageResult == 0;
			break;
		}
		TranslateMessage(&message);
		DispatchMessageW(&message);
	}

	KillTimer(context.waiterWindow, PROPERTY_WINDOW_TIMER_ID);
	DestroyWindow(context.waiterWindow);
	return result;
}

/**
  パスを絶対パスへ変換する
 	@param[in] path 変換対象のパス
 	@param[out] absolutePath 絶対パス
 	@return true:成功 false:失敗
*/
bool GetAbsolutePath(const std::wstring& path, std::wstring& absolutePath)
{
	std::vector<wchar_t> buffer(MAX_PATH);
	for (;;) {
		const DWORD length = GetFullPathNameW(path.c_str(), static_cast<DWORD>(buffer.size()), buffer.data(), nullptr);
		if (length == 0) {
			return false;
		}
		if (length < buffer.size()) {
			absolutePath.assign(buffer.data(), length);
			return true;
		}
		buffer.resize(length + 1);
	}
}

struct CaseInsensitiveLess
{
	bool operator()(const std::wstring& lhs, const std::wstring& rhs) const
	{
		return CompareStringOrdinal(lhs.c_str(), -1, rhs.c_str(), -1, TRUE) == CSTR_LESS_THAN;
	}
};

/**
  指定されたパスのプロパティ表示をシェルへ依頼する
 	@param[in] path 表示対象のパス
 	@return true:表示処理を開始できた false:失敗
*/
bool ShowFileProperties(const std::vector<std::wstring>& paths)
{
	return WaitForPropertyWindow(paths);
}

/**
  コマンドライン引数を引用符付きの形式へ変換する
 	@param[in] argument 変換対象の引数
 	@return 変換後の引数
*/
std::wstring QuoteCommandLineArgument(const std::wstring& argument)
{
	std::wstring result(L"\"");
	size_t backslashCount = 0;
	for (const wchar_t character : argument) {
		if (character == L'\\') {
			++backslashCount;
			continue;
		}

		if (character == L'\"') {
			result.append(backslashCount * 2 + 1, L'\\');
			result.push_back(character);
		}
		else {
			result.append(backslashCount, L'\\');
			result.push_back(character);
		}
		backslashCount = 0;
	}
	result.append(backslashCount * 2, L'\\');
	result.push_back(L'\"');
	return result;
}

/**
  指定されたパスのプロパティ表示用プロセスを起動する
	@param[in] paths 表示対象のパス
	@return true:起動成功 false:起動失敗
*/
bool LaunchPropertyProcess(const std::vector<std::wstring>& paths)
{
	std::vector<wchar_t> modulePathBuffer(MAX_PATH);
	DWORD modulePathLength = 0;
	for (;;) {
		modulePathLength = GetModuleFileNameW(nullptr, modulePathBuffer.data(), static_cast<DWORD>(modulePathBuffer.size()));
		if (modulePathLength == 0) {
			return false;
		}
		if (modulePathLength < modulePathBuffer.size() - 1) {
			break;
		}
		if (modulePathBuffer.size() >= 32768) {
			return false;
		}
		modulePathBuffer.resize(modulePathBuffer.size() * 2);
	}

	const std::wstring modulePath(modulePathBuffer.data(), modulePathLength);
	std::wstring commandLine = QuoteCommandLineArgument(modulePath);
	for (const std::wstring& path : paths) {
		commandLine.push_back(L' ');
		commandLine += QuoteCommandLineArgument(path);
	}
	std::vector<wchar_t> commandLineBuffer(commandLine.begin(), commandLine.end());
	commandLineBuffer.push_back(L'\0');

	STARTUPINFOW startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInformation{};
	const BOOL result = CreateProcessW(
		nullptr,
		commandLineBuffer.data(),
		nullptr,
		nullptr,
		FALSE,
		CREATE_NEW_PROCESS_GROUP,
		nullptr,
		nullptr,
		&startupInfo,
		&processInformation);
	if (result == FALSE) {
		return false;
	}

	CloseHandle(processInformation.hThread);
	CloseHandle(processInformation.hProcess);
	return true;
}

}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
		_In_opt_ HINSTANCE hPrevInstance,
		_In_ LPWSTR    lpCmdLine,
		_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hInstance);
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	int argumentCount = 0;
	LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
	if (arguments == nullptr) {
		return 1;
	}

	bool nowait = false;
	std::set<std::wstring, CaseInsensitiveLess> displayedPaths;
	std::vector<std::wstring> paths;
	for (int i = 1; i < argumentCount; ++i) {
		if (std::wstring(arguments[i]) == L"--nowait") {
			nowait = true;
			continue;
		}

		std::wstring absolutePath;
		if (!GetAbsolutePath(arguments[i], absolutePath)) {
			continue;
		}
		if (!displayedPaths.insert(absolutePath).second) {
			continue;
		}
		paths.push_back(absolutePath);
	}

	if (paths.empty() == false) {
		if (nowait) {
			LaunchPropertyProcess(paths);
		}
		else {
			ShowFileProperties(paths);
		}
	}

	LocalFree(arguments);

	return 0;
}

