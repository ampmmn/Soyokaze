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

	// シェルのコンテキストメニューを扱うためCOMを初期化する
	const HRESULT comResult = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	int argumentCount = 0;
	LPWSTR* arguments = CommandLineToArgvW(GetCommandLineW(), &argumentCount);
	if (arguments == nullptr) {
		if (SUCCEEDED(comResult)) {
			CoUninitialize();
		}
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
	}

	if (SUCCEEDED(comResult)) {
		CoUninitialize();
	}
	return 0;
}