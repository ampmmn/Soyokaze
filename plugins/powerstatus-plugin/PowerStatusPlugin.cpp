#include "pch.h"
#include "PowerStatusPlugin.h"

#define C4_EXCEPTIONS
#define RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS
#define RYML_SINGLE_HDR_DEFINE_NOW
#include "rapidyaml/rapidyaml.hpp"
#include <soyokaze/PluginExportTable.h>

#include <algorithm>
#include <charconv>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <mutex>
#include <string>
#include <vector>

extern HMODULE g_hModule;

namespace {

namespace ryml = c4::yml;

// 監視間隔を指定しない場合の既定値(秒)
constexpr int kDefaultIntervalSeconds = 10;

// 監視間隔として設定できる最大値(秒)
constexpr int kMaxIntervalSeconds = 86400;

// しきい値として設定できる最大値(バッテリー残量の上限と同じ100)
constexpr int kMaxThreshold = 100;

// 監視タイマーの識別子
constexpr UINT_PTR kMonitorTimerId = 1;

// 監視タイマーを受け取るためのメッセージ専用ウインドウのクラス名
const wchar_t kMonitorWindowClass[] = L"SoyokazePowerStatusPluginWindow";

/**
   設定ファイルで定義するしきい値と監視間隔
*/
struct Settings
{
	int upperThreshold{0};
	int lowerThreshold{0};
	int intervalSeconds{kDefaultIntervalSeconds};
};

std::mutex gMutex;                        // 本体側関数テーブルと設定へのアクセスを制御する
LAUNCHER_FUNCTION_TABLE gFunctions{};     // 本体側から受け取った関数テーブル
Settings gSettings;                       // 設定ファイルから読み込んだしきい値
bool gIsSettingsValid{false};             // 設定が有効かどうか(無効な場合は監視しない)
bool gIsUpperNotified{false};             // 上側しきい値を上回ったことを通知済みかどうか
bool gIsLowerNotified{false};             // 下側しきい値を下回ったことを通知済みかどうか
HWND gMonitorWindow{nullptr};             // 監視タイマーを受け取るメッセージ専用ウインドウ

/**
   本体側のログ関数へ整形済みメッセージを出力する
   @param[in] function ログ出力関数
   @param[in] format   printf形式の書式文字列
   @param[in] args     可変長引数
*/
void LogMessage(LPFUNCPRINTMSG function, const char* format, va_list args)
{
	if (function == nullptr || format == nullptr) {
		return;
	}
	va_list lengthArgs;
	va_copy(lengthArgs, args);
	const int length = std::vsnprintf(nullptr, 0, format, lengthArgs);
	va_end(lengthArgs);
	if (length < 0) {
		return;
	}
	std::vector<char> message(static_cast<size_t>(length) + 1);
	std::vsnprintf(message.data(), message.size(), format, args);
	function(message.data());
}

/**
   本体側のログ関数へ可変長引数のメッセージを出力する
   @param[in] member ログ関数メンバーへのポインタ
   @param[in] format printf形式の書式文字列
*/
void Log(LPFUNCPRINTMSG LAUNCHER_FUNCTION_TABLE::*member, const char* format, ...)
{
	LPFUNCPRINTMSG function = nullptr;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		function = gFunctions.*member;
	}
	if (function == nullptr) {
		return;
	}
	va_list args;
	va_start(args, format);
	LogMessage(function, format, args);
	va_end(args);
}

/**
   本体側のトースト通知でメッセージを表示する
   @param[in] format printf形式の書式文字列
*/
void Popup(const char* format, ...)
{
	LPFUNCPRINTMSG popup = nullptr;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		popup = gFunctions.PopupMessage;
	}
	if (popup == nullptr) {
		return;
	}
	va_list args;
	va_start(args, format);
	LogMessage(popup, format, args);
	va_end(args);
}

/**
   UTF-16文字列をUTF-8文字列へ変換する
   @param[in] value UTF-16文字列
   @return 変換後のUTF-8文字列。変換できない場合は空文字列
*/
std::string ToUtf8(const std::wstring& value)
{
	if (value.empty()) {
		return {};
	}
	const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
	if (length <= 0) {
		return {};
	}
	std::string result(static_cast<size_t>(length), '\0');
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), result.data(), length, nullptr, nullptr);
	return result;
}

/**
   APIの文字列取得用バッファへ文字列をコピーする
   @param[in] value  コピー元の文字列
   @param[out] buffer コピー先バッファ
   @param[in] length コピー先バッファの長さ
   @return コピーしたバイト数。lengthが0の場合は必要な長さ
*/
int CopyString(const std::string& value, char* buffer, size_t length)
{
	if (length == 0) {
		return static_cast<int>(value.size() + 1);
	}
	if (buffer == nullptr) {
		return -1;
	}
	const size_t copied = (std::min)(length, value.size() + 1);
	memcpy(buffer, value.c_str(), copied);
	buffer[copied - 1] = '\0';
	return static_cast<int>(copied);
}

/**
   このDLLのファイルパスを取得する
   @return DLLのフルパス。取得できない場合は空
*/
std::filesystem::path GetModulePath()
{
	HMODULE module = g_hModule;
	if (module == nullptr) {
		// DllMainでモジュールハンドルを保持できなかった場合は、このDLL内のアドレスから求める
		GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCWSTR>(&GetModulePath), &module);
	}
	if (module == nullptr) {
		return {};
	}
	wchar_t path[MAX_PATH]{};
	const DWORD length = GetModuleFileNameW(module, path, ARRAYSIZE(path));
	if (length == 0 || length >= ARRAYSIZE(path)) {
		return {};
	}
	return std::filesystem::path(path);
}

/**
   YAMLノードから整数を読み込む
   @param[in] node   読み込み対象ノード
   @param[out] result 読み込んだ整数
   @return 読み込み成功時はtrue
*/
bool ReadInteger(const ryml::ConstNodeRef& node, int& result)
{
	if (!node.readable() || !node.has_val() || node.val_is_null()) {
		return false;
	}
	const c4::csubstr value = node.val();
	const auto parsed = std::from_chars(value.str, value.str + value.len, result);
	return parsed.ec == std::errc() && parsed.ptr == value.str + value.len;
}

/**
   設定ファイルからしきい値と監視間隔を読み込む
   @param[in]  path   設定ファイルのパス
   @param[out] result 読み込んだ設定
   @return 読み込み成功時はtrue
*/
bool LoadSettings(const std::filesystem::path& path, Settings& result)
{
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to open settings file: %s",
			ToUtf8(path.wstring()).c_str());
		return false;
	}
	try {
		const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		// YAML全体をまとめて解析し、各キーを設定へ変換する。
		const auto tree = ryml::parse_in_arena(c4::csubstr(source.data(), source.size()));
		const auto root = tree.crootref();
		if (!root.readable() || !root.is_map()) {
			Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Invalid settings: root element must be a mapping.");
			return false;
		}
		const auto upperKey = c4::to_csubstr("upper-threshold");
		const auto lowerKey = c4::to_csubstr("lower-threshold");
		if (!root.has_child(upperKey) || !root.has_child(lowerKey)) {
			Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog,
				"Invalid settings: 'upper-threshold' and 'lower-threshold' are required.");
			return false;
		}
		if (ReadInteger(root[upperKey], result.upperThreshold) == false ||
			ReadInteger(root[lowerKey], result.lowerThreshold) == false) {
			Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Invalid settings: thresholds must be integers.");
			return false;
		}
		// バッテリー残量は0〜100%なので、範囲外のしきい値は不正として扱う。
		if (result.upperThreshold < 0 || result.upperThreshold > kMaxThreshold ||
			result.lowerThreshold < 0 || result.lowerThreshold > kMaxThreshold) {
			Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog,
				"Invalid settings: thresholds must be between 0 and %d.", kMaxThreshold);
			return false;
		}
		if (result.upperThreshold <= result.lowerThreshold) {
			Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog,
				"Invalid settings: 'upper-threshold' must be greater than 'lower-threshold'.");
			return false;
		}
		result.intervalSeconds = kDefaultIntervalSeconds;
		const auto intervalKey = c4::to_csubstr("interval-seconds");
		if (root.has_child(intervalKey)) {
			if (ReadInteger(root[intervalKey], result.intervalSeconds) == false) {
				Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog,
					"Invalid settings: 'interval-seconds' must be an integer.");
				return false;
			}
			if (result.intervalSeconds <= 0 || result.intervalSeconds > kMaxIntervalSeconds) {
				Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog,
					"Invalid settings: 'interval-seconds' must be between 1 and %d.", kMaxIntervalSeconds);
				return false;
			}
		}
	}
	catch (const std::exception& exception) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to parse settings: %s", exception.what());
		return false;
	}
	return true;
}

/**
   現在の電源状態を取得し、しきい値を跨いだときに通知する
   @param[in] notify 通知を行うか。起動時の状態記録ではfalseを指定する
*/
void UpdatePowerStatus(bool notify)
{
	SYSTEM_POWER_STATUS status{};
	if (GetSystemPowerStatus(&status) == FALSE) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "GetSystemPowerStatus failed. error:%lu", GetLastError());
		return;
	}
	// 残量が未知(255)の場合は判定できないため何もしない
	if (status.BatteryLifePercent > kMaxThreshold) {
		return;
	}

	Settings settings;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		// 設定が無効な状態では監視しない
		if (gIsSettingsValid == false) {
			return;
		}
		settings = gSettings;
	}
	const int percent = status.BatteryLifePercent;

	// 上側しきい値を上回ったら通知する。下回るまでは再通知しない。
	if (settings.upperThreshold < percent) {
		if (notify && gIsUpperNotified == false) {
			Popup("バッテリー残量が %d%% になりました(上側しきい値 %d%% を上回りました)",
				percent, settings.upperThreshold);
		}
		gIsUpperNotified = true;
	}
	else {
		gIsUpperNotified = false;
	}

	// 下側しきい値を下回ったら通知する。上回るまでは再通知しない。
	if (percent < settings.lowerThreshold) {
		if (notify && gIsLowerNotified == false) {
			Popup("バッテリー残量が %d%% になりました(下側しきい値 %d%% を下回りました)",
				percent, settings.lowerThreshold);
		}
		gIsLowerNotified = true;
	}
	else {
		gIsLowerNotified = false;
	}
}

/**
   監視タイマーを受け取るためのメッセージ専用ウインドウのプロシージャ
   @param[in] window  ウインドウハンドル
   @param[in] message メッセージ
   @param[in] wParam  メッセージの追加情報
   @param[in] lParam  メッセージの追加情報
   @return メッセージ処理結果
*/
LRESULT CALLBACK MonitorWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_TIMER:
		// タイマーは本体のメインスレッド上で処理されるため、本体APIをそのまま呼び出せる
		if (wParam == kMonitorTimerId) {
			UpdatePowerStatus(true);
		}
		return 0;
	default:
		break;
	}
	return DefWindowProcW(window, message, wParam, lParam);
}

/**
   監視用のメッセージ専用ウインドウを作成し、タイマーによる監視を開始する
   @param[in] settings 監視設定
   @return 成功時はtrue
*/
bool StartMonitor(const Settings& settings)
{
	const HINSTANCE instance = reinterpret_cast<HINSTANCE>(g_hModule);
	if (instance == nullptr) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to get module handle.");
		return false;
	}

	WNDCLASSEXW wcex{};
	wcex.cbSize = sizeof(wcex);
	wcex.lpfnWndProc = &MonitorWindowProc;
	wcex.hInstance = instance;
	wcex.lpszClassName = kMonitorWindowClass;
	// すでに登録済みの場合はエラーになるが、ウインドウ作成には影響しないため無視する
	RegisterClassExW(&wcex);

	// 本体のウインドウとメッセージが衝突しないよう、メッセージ専用ウインドウでタイマーを受け取る
	HWND window = CreateWindowExW(0, kMonitorWindowClass, nullptr, 0, 0, 0, 0, 0, HWND_MESSAGE,
		nullptr, instance, nullptr);
	if (window == nullptr) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to create monitor window. error:%lu", GetLastError());
		return false;
	}
	if (SetTimer(window, kMonitorTimerId, static_cast<UINT>(settings.intervalSeconds) * 1000, nullptr) == 0) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to start monitor timer. error:%lu", GetLastError());
		DestroyWindow(window);
		return false;
	}
	gMonitorWindow = window;

	// 起動時点ですでにしきい値を跨いでいた場合に誤通知しないよう、初期状態だけを記録する
	UpdatePowerStatus(false);
	return true;
}

/**
   監視タイマーを停止し、メッセージ専用ウインドウを破棄する
*/
void StopMonitor()
{
	if (gMonitorWindow == nullptr) {
		return;
	}
	KillTimer(gMonitorWindow, kMonitorTimerId);
	DestroyWindow(gMonitorWindow);
	gMonitorWindow = nullptr;
	gIsUpperNotified = false;
	gIsLowerNotified = false;
}

} // 名前空間

namespace powerstatus {

int Initialize(LAUNCHER_FUNCTION_TABLE* table, const char** pluginInfo)
{
	if (table == nullptr || pluginInfo == nullptr) {
		return 1;
	}
	{
		std::lock_guard<std::mutex> lock(gMutex);
		// 初期化後も本体側APIを利用できるよう関数テーブルをコピーして保持する。
		gFunctions = *table;
	}
	// プラグインのメタデータはDLLのアンロードまで有効な静的文字列で返す。
	static constexpr char info[] = R"json({
  "displayName": "powerstatus-plugin",
  "pluginId": "F3B4BCCB-CB04-40B3-8E16-3AABFDE2A186",
  "pluginVersion": "0.1.0",
  "pluginApiVersion": 102,
  "pluginDescription": "バッテリー残量を監視し、しきい値を上下回った場合に通知する",
  "pluginDeveloper": "ampmmn",
  "pluginLicenseName": "MIT License",
  "url": "https://github.com/ampmmn/Soyokaze/tree/main/plugins/powerstatus-plugin"
})json";
	*pluginInfo = info;

	// 設定ファイルはDLLと同じディレクトリに置く
	const auto settingsPath = GetModulePath().parent_path() / L"powerstatus.yaml";
	Settings settings;
	if (LoadSettings(settingsPath, settings) == false) {
		// 設定が読み込めない場合は監視を開始しない(初期化自体は成功とする)
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Power status monitoring is disabled.");
		return 0;
	}
	{
		std::lock_guard<std::mutex> lock(gMutex);
		gSettings = settings;
		gIsSettingsValid = true;
	}
	if (StartMonitor(settings) == false) {
		StopMonitor();
		std::lock_guard<std::mutex> lock(gMutex);
		gSettings = Settings();
		gIsSettingsValid = false;
		return 0;
	}
	Log(&LAUNCHER_FUNCTION_TABLE::InfoLog,
		"Power status monitoring started. upper-threshold:%d%%, lower-threshold:%d%%, interval:%ds",
		settings.upperThreshold, settings.lowerThreshold, settings.intervalSeconds);
	return 0;
}

void Finalize()
{
	StopMonitor();
	std::lock_guard<std::mutex> lock(gMutex);
	gSettings = Settings();
	gIsSettingsValid = false;
	gFunctions = LAUNCHER_FUNCTION_TABLE();
}

} // powerstatus名前空間

/**
   本体へpowerstatusプラグインのエクスポート関数テーブルを渡す
   @param[in]  version 本体が要求するプラグインAPIバージョン
   @param[out] table   エクスポート関数テーブル
   @return 成功時は0、未対応バージョンまたは不正な引数の場合は1
*/
extern "C" int LNCRPLUGIN_API LNCRPLUGIN_Bind(int version, LNCRPLUGIN_EXPORTTABLE* table)
{
	if (version != PLUGINVERSION || table == nullptr) {
		return 1;
	}
	table->Initialize = &powerstatus::Initialize;
	table->Finalize = &powerstatus::Finalize;
	// このプラグインはキーワード検索を行わないため、検索系の関数は空実装とする。
	table->Query = [](void*, MATCHER_FUNCTION_TABLE*) { return static_cast<LNCRPLUGINMATCHHANDLE>(nullptr); };
	table->GetMatchCount = [](LNCRPLUGINMATCHHANDLE) { return 0; };
	table->GetMatchLevel = [](LNCRPLUGINMATCHHANDLE, int) { return -1; };
	table->CloseHandle = [](LNCRPLUGINMATCHHANDLE) {};
	table->GetName = [](LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length) {
		return CopyString("", buffer, length);
	};
	table->GetDescription = [](LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length) {
		return CopyString("", buffer, length);
	};
	table->GetGuide = [](LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length) {
		return CopyString("", buffer, length);
	};
	table->GetTypeDisplayName = [](LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length) {
		return CopyString("PowerStatus", buffer, length);
	};
	table->CanExecute = [](LNCRPLUGINMATCHHANDLE, int) { return 0; };
	table->Execute = [](LNCRPLUGINMATCHHANDLE, int, int, char**) { return 1; };
	table->GetErrorString = [](LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length) {
		return CopyString("", buffer, length);
	};
	table->GetIcon = [](LNCRPLUGINMATCHHANDLE, int, HICON*) { return 1; };
	return 0;
}
