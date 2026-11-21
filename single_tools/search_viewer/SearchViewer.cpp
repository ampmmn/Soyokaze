#include "SearchViewerConfig.h"
#include "SearchViewerCommandLine.h"
#include "SearchViewerWindowPlacement.h"

#include <Windows.h>
#include <Shellapi.h>
#include <wrl.h>
#include <WebView2.h>
#include <WebView2EnvironmentOptions.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#pragma comment(lib, "shell32.lib")

using Microsoft::WRL::Callback;
using Microsoft::WRL::ComPtr;

namespace {

constexpr wchar_t WINDOW_CLASS_PREFIX[] = L"Soyokaze.SearchViewer.";
constexpr wchar_t MUTEX_PREFIX[] = L"Local\\Soyokaze.SearchViewer.";
constexpr ULONG_PTR IPC_TAG = 0x53565731;
constexpr UINT WM_NAVIGATE_PENDING = WM_APP + 41;
constexpr LRESULT IPC_ACCEPTED = 1;
constexpr LRESULT IPC_CONFIG_MISMATCH = 2;
constexpr LRESULT IPC_REJECTED = 3;

struct IpcHeader
{
	uint32_t version;
	uint32_t configLength;
	uint32_t queryLength;
};

/** 実行ファイルの配置ディレクトリを取得する */
bool GetModuleDirectory(std::wstring& directory)
{
	std::vector<wchar_t> buffer(32768);
	const DWORD length = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
	if (length == 0 || length >= buffer.size()) {
		return false;
	}
	std::filesystem::path path(std::wstring(buffer.data(), length));
	directory = path.parent_path().wstring();
	return directory.empty() == false;
}

/** 指定パスを絶対パスへ変換する */
bool GetAbsolutePath(const std::wstring& path, std::wstring& absolutePath)
{
	std::vector<wchar_t> buffer(32768);
	const DWORD length = GetFullPathNameW(path.c_str(), static_cast<DWORD>(buffer.size()), buffer.data(), nullptr);
	if (length == 0 || length >= buffer.size()) {
		return false;
	}
	absolutePath.assign(buffer.data(), length);
	return true;
}

/** プロファイルごとのウインドウクラス名を生成する */
std::wstring GetWindowClassName(const std::wstring& profile)
{
	return std::wstring(WINDOW_CLASS_PREFIX) + profile;
}

/** プロファイルごとの排他オブジェクト名を生成する */
std::wstring GetMutexName(const std::wstring& profile)
{
	return std::wstring(MUTEX_PREFIX) + profile;
}

/** Windowsのパス比較として大文字小文字を無視して比較する */
bool IsSamePath(const std::wstring& lhs, const std::wstring& rhs)
{
	return CompareStringOrdinal(lhs.c_str(), -1, rhs.c_str(), -1, TRUE) == CSTR_EQUAL;
}

/** ファイルをバイナリモードで読み込む */
std::string ReadTextFile(const std::wstring& path)
{
	std::ifstream file(path, std::ios::binary);
	if (file.is_open() == false) {
		return {};
	}
	return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
}

/** ファイルをバイナリモードで書き出す */
bool WriteTextFile(const std::wstring& path, const std::string& contents)
{
	std::ofstream file(path, std::ios::binary | std::ios::trunc);
	if (file.is_open() == false) {
		return false;
	}
	file.write(contents.data(), static_cast<std::streamsize>(contents.size()));
	return file.good();
}

/** 指定プロファイルごとに検索ウインドウのプロセスを起動する */
bool LaunchProfileInstances(const SearchViewerCommandLine::Options& options, const std::wstring& moduleDirectory, std::wstring& error)
{
	const std::wstring executablePath = (std::filesystem::path(moduleDirectory) / L"SearchViewer.exe").wstring();
	for (const auto& profile : options.profiles) {
		std::wstring commandLine = SearchViewerCommandLine::QuoteArgument(executablePath);
		commandLine += L" --profile ";
		commandLine += SearchViewerCommandLine::QuoteArgument(profile);
		commandLine += L" --config ";
		commandLine += SearchViewerCommandLine::QuoteArgument(options.configPath);
		commandLine.push_back(L' ');
		commandLine += SearchViewerCommandLine::QuoteArgument(options.query);

		std::vector<wchar_t> mutableCommandLine(commandLine.begin(), commandLine.end());
		mutableCommandLine.push_back(L'\0');
		STARTUPINFOW startupInfo{};
		startupInfo.cb = sizeof(startupInfo);
		PROCESS_INFORMATION processInfo{};
		if (CreateProcessW(executablePath.c_str(), mutableCommandLine.data(), nullptr, nullptr, FALSE, 0, nullptr, moduleDirectory.c_str(), &startupInfo, &processInfo) == FALSE) {
			error = L"プロファイルの検索ウインドウを起動できません: " + profile;
			return false;
		}
		CloseHandle(processInfo.hThread);
		CloseHandle(processInfo.hProcess);
	}
	return true;
}

/** エラーメッセージを表示する */
void ShowError(const std::wstring& message)
{
	MessageBoxW(nullptr, message.c_str(), L"SearchViewer", MB_OK | MB_ICONERROR);
}

} // 名前空間

class SearchViewerWindow
{
public:
	struct WebState
	{
		ComPtr<ICoreWebView2Controller> controller;
		ComPtr<ICoreWebView2> webview;
		HWND hwnd = nullptr;
		HWND addressBar = nullptr;
		int addressBarHeight = 28;
		std::wstring pendingUrl;
	};

	struct PImpl
	{
		HINSTANCE instance = nullptr;
		HWND hwnd = nullptr;
		HWND addressBar = nullptr;
		HFONT addressBarFont = nullptr;
		std::wstring profile;
		std::wstring configPath;
		std::wstring moduleDirectory;
		std::wstring windowClassName;
		std::wstring title;
		std::wstring placementPath;
		SearchViewerConfig config;
		std::shared_ptr<WebState> webState = std::make_shared<WebState>();
	};

	explicit SearchViewerWindow(HINSTANCE instance) : in(std::make_unique<PImpl>())
	{
		in->instance = instance;
	}

	/** WebView2を終了し、保持中のCOMオブジェクトを解放する */
	~SearchViewerWindow()
	{
		if (in->webState->controller) {
			in->webState->controller->Close();
		}
		if (in->addressBarFont != nullptr) {
			DeleteObject(in->addressBarFont);
		}
	}

	/** メインウインドウを作成し、WebView2の初期化を開始する */
	bool Create(const SearchViewerCommandLine::Options& options, const std::wstring& configPath, const std::wstring& moduleDirectory, SearchViewerConfig config, const std::wstring& initialUrl)
	{
		in->profile = options.profiles.front();
		in->configPath = configPath;
		in->moduleDirectory = moduleDirectory;
		in->config = std::move(config);
		in->webState->pendingUrl = initialUrl;
		in->windowClassName = GetWindowClassName(in->profile);
		in->config.GetTitle(in->profile, in->title);
		in->placementPath = (std::filesystem::path(moduleDirectory) / L"SearchViewerWindow.yaml").wstring();

		WNDCLASSEXW windowClass{};
		windowClass.cbSize = sizeof(windowClass);
		windowClass.hInstance = in->instance;
		windowClass.lpfnWndProc = WindowProc;
		windowClass.hCursor = LoadCursorW(nullptr, IDC_ARROW);
		windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
		windowClass.lpszClassName = in->windowClassName.c_str();
		if (RegisterClassExW(&windowClass) == 0) {
			return false;
		}

		in->hwnd = CreateWindowExW(0, in->windowClassName.c_str(), in->title.c_str(), WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, 1100, 800, nullptr, nullptr, in->instance, in.get());
		if (in->hwnd == nullptr) {
			return false;
		}
		ShowWindow(in->hwnd, SW_SHOWNORMAL);
		UpdateWindow(in->hwnd);
		RestorePlacement(in.get());
		InitializeWebView();
		return true;
	}

	/** ウインドウメッセージを処理する */
	int Run()
	{
		MSG message{};
		while (GetMessageW(&message, nullptr, 0, 0) > 0) {
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}
		return static_cast<int>(message.wParam);
	}

private:
	/** タイトルバーのキャプションフォントをアドレス欄へ設定する */
	static void ConfigureAddressBarFont(PImpl* state)
	{
		if (state->addressBar == nullptr) {
			return;
		}

		NONCLIENTMETRICSW nonClientMetrics{};
		nonClientMetrics.cbSize = sizeof(nonClientMetrics);
		if (SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, static_cast<UINT>(sizeof(nonClientMetrics)), &nonClientMetrics, 0) != FALSE) {
			state->addressBarFont = CreateFontIndirectW(&nonClientMetrics.lfCaptionFont);
		}
		if (state->addressBarFont != nullptr) {
			SendMessageW(state->addressBar, WM_SETFONT, reinterpret_cast<WPARAM>(state->addressBarFont), TRUE);
		}

		HFONT font = reinterpret_cast<HFONT>(SendMessageW(state->addressBar, WM_GETFONT, 0, 0));
		if (font == nullptr) {
			font = static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));
		}
		HDC deviceContext = GetDC(state->addressBar);
		if (deviceContext == nullptr) {
			return;
		}
		const HGDIOBJ previousFont = SelectObject(deviceContext, font);
		TEXTMETRICW textMetrics{};
		if (previousFont != nullptr && previousFont != HGDI_ERROR && GetTextMetricsW(deviceContext, &textMetrics) != FALSE) {
			state->webState->addressBarHeight = (std::max)(28, static_cast<int>(textMetrics.tmHeight) + 8);
		}
		if (previousFont != nullptr && previousFont != HGDI_ERROR) {
			SelectObject(deviceContext, previousFont);
		}
		ReleaseDC(state->addressBar, deviceContext);
	}

	/** メインウインドウのメッセージを処理する */
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		PImpl* state = reinterpret_cast<PImpl*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (message == WM_NCCREATE) {
			auto create = reinterpret_cast<CREATESTRUCTW*>(lParam);
			state = static_cast<PImpl*>(create->lpCreateParams);
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
			state->hwnd = hwnd;
		}
		if (state == nullptr) {
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}

		switch (message) {
		case WM_CREATE:
			state->addressBar = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_READONLY,
				0, 0, 0, 0, hwnd, reinterpret_cast<HMENU>(1001), state->instance, nullptr);
			ConfigureAddressBarFont(state);
			state->webState->hwnd = hwnd;
			state->webState->addressBar = state->addressBar;
			return 0;
		case WM_SIZE:
			Layout(state, LOWORD(lParam), HIWORD(lParam));
			return 0;
		case WM_COPYDATA:
			return HandleCopyData(state, reinterpret_cast<const COPYDATASTRUCT*>(lParam));
		case WM_NAVIGATE_PENDING:
			if (state->webState->webview && state->webState->pendingUrl.empty() == false) {
				state->webState->webview->Navigate(state->webState->pendingUrl.c_str());
				state->webState->pendingUrl.clear();
			}
			return 0;
		case WM_CLOSE:
			SavePlacement(state);
			DestroyWindow(hwnd);
			return 0;
		case WM_QUERYENDSESSION:
			SavePlacement(state);
			return TRUE;
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;
		case WM_NCDESTROY:
			if (state->addressBarFont != nullptr) {
				DeleteObject(state->addressBarFont);
				state->addressBarFont = nullptr;
			}
			SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
			state->hwnd = nullptr;
			return DefWindowProcW(hwnd, message, wParam, lParam);
		default:
			return DefWindowProcW(hwnd, message, wParam, lParam);
		}
	}

	/** 読み取り専用アドレス欄とWebView2の表示範囲を更新する */
	static void Layout(PImpl* state, int width, int height)
	{
		if (state->addressBar != nullptr) {
			MoveWindow(state->addressBar, 0, 0, width, state->webState->addressBarHeight, TRUE);
		}
		if (state->webState->controller) {
			RECT bounds{ 0, state->webState->addressBarHeight, width, height };
			state->webState->controller->put_Bounds(bounds);
		}
	}

	/** 保存済みのウインドウ位置を復元する */
	static void RestorePlacement(PImpl* state)
	{
		WINDOWPLACEMENT placement{};
		placement.length = sizeof(placement);
		placement.showCmd = SW_SHOWNORMAL;
		placement.rcNormalPosition = RECT{ 100, 100, 1200, 900 };
		const std::string yaml = ReadTextFile(state->placementPath);
		if (SearchViewerWindowPlacement::Parse(yaml, state->profile, placement) == false) {
			return;
		}
		if (placement.rcNormalPosition.right <= placement.rcNormalPosition.left || placement.rcNormalPosition.bottom <= placement.rcNormalPosition.top) {
			return;
		}
		SetWindowPlacement(state->hwnd, &placement);
	}

	/** 現在のウインドウ位置を設定ファイルへ保存する */
	static void SavePlacement(PImpl* state)
	{
		WINDOWPLACEMENT placement{};
		placement.length = sizeof(placement);
		if (GetWindowPlacement(state->hwnd, &placement) == FALSE) {
			return;
		}
		const std::string yaml = SearchViewerWindowPlacement::Update(ReadTextFile(state->placementPath), state->profile, placement);
		if (yaml.empty() == false) {
			WriteTextFile(state->placementPath, yaml);
		}
	}

	/** 後続プロセスからの検索要求を検証して処理する */
	static LRESULT HandleCopyData(PImpl* state, const COPYDATASTRUCT* data)
	{
		if (data == nullptr || data->lpData == nullptr || data->dwData != IPC_TAG || data->cbData < sizeof(IpcHeader)) {
			return IPC_REJECTED;
		}
		const auto header = static_cast<const IpcHeader*>(data->lpData);
		const uint64_t expectedSize = sizeof(IpcHeader) + (static_cast<uint64_t>(header->configLength) + header->queryLength) * sizeof(wchar_t);
		if (header->version != 1 || expectedSize != data->cbData || header->configLength > 32768 || header->queryLength > 32768) {
			return IPC_REJECTED;
		}
		const wchar_t* strings = reinterpret_cast<const wchar_t*>(header + 1);
		const std::wstring configPath(strings, header->configLength);
		const std::wstring query(strings + header->configLength, header->queryLength);
		if (IsSamePath(configPath, state->configPath) == false) {
			return IPC_CONFIG_MISMATCH;
		}

		std::wstring url;
		std::wstring error;
		if (state->config.BuildSearchUrl(state->profile, query, url, error) == false) {
			return IPC_REJECTED;
		}
		state->webState->pendingUrl = std::move(url);
		if (state->webState->webview) {
			PostMessageW(state->hwnd, WM_NAVIGATE_PENDING, 0, 0);
		}
		ShowWindow(state->hwnd, IsIconic(state->hwnd) ? SW_RESTORE : SW_SHOW);
		SetForegroundWindow(state->hwnd);
		return IPC_ACCEPTED;
	}

	/** プロファイル専用の保存先でWebView2を初期化する */
	void InitializeWebView()
	{
		const auto state = in->webState;
		const std::wstring dataFolder = (std::filesystem::path(in->moduleDirectory) / L"UserData" / in->profile).wstring();
		std::error_code filesystemError;
		std::filesystem::create_directories(dataFolder, filesystemError);
		if (filesystemError) {
			ShowError(L"WebView2のデータ保存先を作成できません: " + dataFolder);
			PostMessageW(in->hwnd, WM_CLOSE, 0, 0);
			return;
		}

		const std::weak_ptr<WebState> weakState(state);
		auto options = Microsoft::WRL::Make<CoreWebView2EnvironmentOptions>();
		CreateCoreWebView2EnvironmentWithOptions(nullptr, dataFolder.c_str(), options.Get(),
			Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
				[weakState](HRESULT result, ICoreWebView2Environment* environment) -> HRESULT {
					auto state = weakState.lock();
					if (state == nullptr || state->hwnd == nullptr) {
						return S_OK;
					}
					if (FAILED(result) || environment == nullptr) {
						MessageBoxW(state->hwnd, L"WebView2を初期化できません。", L"SearchViewer", MB_OK | MB_ICONERROR);
						PostMessageW(state->hwnd, WM_CLOSE, 0, 0);
						return FAILED(result) ? result : E_FAIL;
					}
					environment->CreateCoreWebView2Controller(state->hwnd,
						Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
							[weakState](HRESULT controllerResult, ICoreWebView2Controller* controller) -> HRESULT {
								auto state = weakState.lock();
								if (state == nullptr || state->hwnd == nullptr) {
									return S_OK;
								}
								if (FAILED(controllerResult) || controller == nullptr) {
									MessageBoxW(state->hwnd, L"WebView2の表示領域を作成できません。", L"SearchViewer", MB_OK | MB_ICONERROR);
									PostMessageW(state->hwnd, WM_CLOSE, 0, 0);
									return FAILED(controllerResult) ? controllerResult : E_FAIL;
								}
								state->controller = controller;
								if (FAILED(state->controller->get_CoreWebView2(&state->webview)) || state->webview == nullptr) {
									state->controller->Close();
									state->controller.Reset();
									return E_FAIL;
								}
								RECT bounds{};
								GetClientRect(state->hwnd, &bounds);
								bounds.top = state->addressBarHeight;
								state->controller->put_Bounds(bounds);
								ComPtr<ICoreWebView2Settings> settings;
								state->webview->get_Settings(&settings);
								if (settings) {
									settings->put_AreDevToolsEnabled(FALSE);
									settings->put_AreHostObjectsAllowed(FALSE);
									settings->put_IsWebMessageEnabled(FALSE);
										ComPtr<ICoreWebView2Settings3> settings3;
										if (SUCCEEDED(settings.As(&settings3)) && settings3) {
											settings3->put_AreBrowserAcceleratorKeysEnabled(TRUE);
										}
								}
								state->webview->add_NavigationStarting(
									Callback<ICoreWebView2NavigationStartingEventHandler>(
										[weakState](ICoreWebView2*, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
											LPWSTR uri = nullptr;
											if (SUCCEEDED(args->get_Uri(&uri)) && uri != nullptr) {
												if (SearchViewerConfig::IsAllowedUrl(uri) == false) {
													args->put_Cancel(TRUE);
												}
												else if (auto state = weakState.lock()) {
													SetWindowTextW(state->addressBar, uri);
												}
												CoTaskMemFree(uri);
											}
											return S_OK;
										}).Get(), nullptr);
								state->webview->add_NewWindowRequested(
									Callback<ICoreWebView2NewWindowRequestedEventHandler>(
										[weakState](ICoreWebView2*, ICoreWebView2NewWindowRequestedEventArgs* args) -> HRESULT {
											args->put_Handled(TRUE);
											LPWSTR uri = nullptr;
											if (SUCCEEDED(args->get_Uri(&uri)) && uri != nullptr) {
												if (SearchViewerConfig::IsAllowedUrl(uri)) {
													if (auto state = weakState.lock()) {
														state->webview->Navigate(uri);
													}
												}
												CoTaskMemFree(uri);
												}
											return S_OK;
										}).Get(), nullptr);
								PostMessageW(state->hwnd, WM_NAVIGATE_PENDING, 0, 0);
								return S_OK;
							}).Get());
					return S_OK;
				}).Get());
	}

	std::unique_ptr<PImpl> in;
};

namespace {

/** 起動済みウインドウへ設定パスと検索語を送信する */
LRESULT SendToExistingInstance(const std::wstring& profile, const std::wstring& configPath, const std::wstring& query)
{
	const std::wstring className = GetWindowClassName(profile);
	HWND target = nullptr;
	for (int attempt = 0; attempt < 40 && target == nullptr; ++attempt) {
		target = FindWindowW(className.c_str(), nullptr);
		if (target == nullptr) {
			Sleep(50);
		}
	}
	if (target == nullptr) {
		return 0;
	}
	if (configPath.size() > 32768 || query.size() > 32768) {
		return IPC_REJECTED;
	}
	IpcHeader header{ 1, static_cast<uint32_t>(configPath.size()), static_cast<uint32_t>(query.size()) };
	std::vector<BYTE> payload(sizeof(header) + (configPath.size() + query.size()) * sizeof(wchar_t));
	memcpy(payload.data(), &header, sizeof(header));
	wchar_t* strings = reinterpret_cast<wchar_t*>(payload.data() + sizeof(header));
	memcpy(strings, configPath.data(), configPath.size() * sizeof(wchar_t));
	memcpy(strings + configPath.size(), query.data(), query.size() * sizeof(wchar_t));
	COPYDATASTRUCT data{};
	data.dwData = IPC_TAG;
	data.cbData = static_cast<DWORD>(payload.size());
	data.lpData = payload.data();
	DWORD_PTR response = 0;
	if (SendMessageTimeoutW(target, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&data), SMTO_ABORTIFHUNG | SMTO_BLOCK, 3000, &response) == 0) {
		return 0;
	}
	return static_cast<LRESULT>(response);
}

} // 名前空間

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int)
{
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv == nullptr) {
		ShowError(L"コマンドラインを解析できません。");
		return 1;
	}
	std::vector<std::wstring> arguments;
	for (int i = 1; i < argc; ++i) {
		arguments.emplace_back(argv[i]);
	}
	LocalFree(argv);

	SearchViewerCommandLine::Options options;
	std::wstring error;
	if (SearchViewerCommandLine::Parse(arguments, options, error) == false) {
		ShowError(error);
		return 1;
	}

	std::wstring moduleDirectory;
	if (GetModuleDirectory(moduleDirectory) == false) {
		ShowError(L"実行ファイルの場所を取得できません。" );
		return 1;
	}
	if (options.configPath.empty()) {
		options.configPath = (std::filesystem::path(moduleDirectory) / L"SearchViewer.yaml").wstring();
	}
	std::wstring absoluteConfigPath;
	if (GetAbsolutePath(options.configPath, absoluteConfigPath) == false) {
		ShowError(L"設定ファイルの絶対パスを取得できません。" );
		return 1;
	}
	options.configPath = absoluteConfigPath;

	SearchViewerConfig config;
	if (SearchViewerConfig::LoadFile(options.configPath, config, error) == false) {
		ShowError(error);
		return 1;
	}
	std::vector<std::wstring> initialUrls;
	for (const auto& profile : options.profiles) {
		std::wstring title;
		if (config.GetTitle(profile, title) == false) {
			ShowError(L"Unknown profile: " + profile);
			return 1;
		}
		std::wstring url;
		if (config.BuildSearchUrl(profile, options.query, url, error) == false) {
			ShowError(error);
			return 1;
		}
		initialUrls.push_back(std::move(url));
	}

	if (options.profiles.size() > 1) {
		if (LaunchProfileInstances(options, moduleDirectory, error) == false) {
			ShowError(error);
			return 1;
		}
		return 0;
	}
	const std::wstring& profile = options.profiles.front();

	HANDLE mutex = CreateMutexW(nullptr, TRUE, GetMutexName(profile).c_str());
	if (mutex == nullptr) {
		ShowError(L"単一インスタンスの確認に失敗しました。" );
		return 1;
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		CloseHandle(mutex);
		const LRESULT result = SendToExistingInstance(profile, options.configPath, options.query);
		if (result == IPC_CONFIG_MISMATCH) {
			ShowError(L"起動済み SearchViewer と異なる設定ファイルが指定されています。" );
			return 1;
		}
		if (result != IPC_ACCEPTED) {
			ShowError(L"起動済み SearchViewer へ検索要求を送信できません。" );
			return 1;
		}
		return 0;
	}

	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(comResult)) {
		CloseHandle(mutex);
		ShowError(L"COMを初期化できません。" );
		return 1;
	}
	int result = 1;
	{
		SearchViewerWindow window(instance);
		if (window.Create(options, options.configPath, moduleDirectory, std::move(config), initialUrls.front()) == false) {
			ShowError(L"メインウインドウを作成できません。" );
		}
		else {
			result = window.Run();
		}
	}
	CoUninitialize();
	CloseHandle(mutex);
	return result;
}
