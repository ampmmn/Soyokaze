// contextmenu.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "contextmenu.h"

#include "CommandLineOptions.h"
#include "ShellContextMenu.h"

#include <ole2.h>
#include <shellapi.h>

#include <set>
#include <string>
#include <vector>

#pragma comment(lib, "Shell32.lib")

namespace {

struct CaseInsensitiveLess
{
	bool operator()(const std::wstring& lhs, const std::wstring& rhs) const
	{
		return CompareStringOrdinal(lhs.c_str(), -1, rhs.c_str(), -1, TRUE) == CSTR_LESS_THAN;
	}
};

/**
  パスを絶対パスへ変換する
  @param[in]  path         変換対象のパス
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

/**
  パスが存在するかを確認する
  @param[in] path 確認するパス
  @return true:存在する false:存在しない
*/
bool PathExists(const std::wstring& path)
{
	const DWORD attributes = GetFileAttributesW(path.c_str());
	return attributes != INVALID_FILE_ATTRIBUTES;
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
  コンテキストメニュー表示用の後続プロセスを起動する
  @param[in] paths   対象のパス
  @param[in] options コマンドラインオプション
  @return true:起動成功 false:起動失敗
*/
bool LaunchContextMenuProcess(const std::vector<std::wstring>& paths, const CommandLineOptions& options)
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
	if (options.HasPosition()) {
		const POINT position = options.GetPosition();
		commandLine += L" --position=";
		commandLine += std::to_wstring(position.x);
		commandLine.push_back(L',');
		commandLine += std::to_wstring(position.y);
	}
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

} // 名前空間

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

	// コマンドライン引数を解析する
	CommandLineOptions options;
	if (argumentCount > 1) {
		options.Parse(argumentCount - 1, arguments + 1);
	}
	LocalFree(arguments);

	// 存在しないパスは対象から除外する
	std::set<std::wstring, CaseInsensitiveLess> uniquePaths;
	std::vector<std::wstring> paths;
	for (const std::wstring& path : options.GetPaths()) {
		std::wstring absolutePath;
		if (!GetAbsolutePath(path, absolutePath)) {
			continue;
		}
		if (!PathExists(absolutePath)) {
			// 存在しないパスの場合は何もしない
			continue;
		}
		if (!uniquePaths.insert(absolutePath).second) {
			continue;
		}
		paths.push_back(absolutePath);
	}

	if (paths.empty() == false) {
		if (options.HasNowait()) {
			// メニュー表示は後続プロセスに任せ、呼び出し元は待たずに終了する
			LaunchContextMenuProcess(paths, options);
		}
		else {
			// シェルのコンテキストメニューを扱うためCOMを初期化する
			const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

			// 表示位置を確定する
			POINT position{ 0, 0 };
			if (options.HasPosition()) {
				position = options.GetPosition();
			}
			else {
				// 指定がない場合はマウスカーソルの位置を使う
				GetCursorPos(&position);
			}

			ShellContextMenu contextMenu;
			contextMenu.Show(paths, position);

			if (SUCCEEDED(comResult)) {
				CoUninitialize();
			}
		}
	}
	return 0;
}
