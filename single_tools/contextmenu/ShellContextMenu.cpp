#include "framework.h"
#include "ShellContextMenu.h"

#include <shellapi.h>
#include <shlobj.h>
#include <shobjidl.h>

#include <string>
#include <vector>

#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Ole32.lib")

namespace {

// メニューへ割り当てるコマンドIDの範囲
constexpr UINT MENU_COMMAND_ID_FIRST = 0x8000;
constexpr UINT MENU_COMMAND_ID_LAST = 0x8FFF;

// コマンド実行後、シェル側の処理が完了するのを待ち合わせる時間
// (ウインドウが出現しなかった場合の猶予時間、待ち合わせの上限、監視間隔)
// シェルは一部verbの処理を非同期に行うため、呼び出し元プロセスが
// 終了すると処理が取り消される。猶予時間はシェルからの呼び出しを
// 受け取れるだけの余裕を持たせている。
constexpr DWORD COMMAND_WAIT_GRACE_PERIOD = 4000;
constexpr DWORD COMMAND_WAIT_TIMEOUT = 60000;
constexpr DWORD COMMAND_WAIT_INTERVAL = 100;

// 待ち合わせ用タイマーのID
constexpr UINT_PTR COMMAND_TIMER_ID = 1;

// メニューの所有ウィンドウのクラス名
const wchar_t OWNER_WINDOW_CLASS_NAME[] = L"SoyokazeContextMenuOwner";

/**
  コンテキストメニュー表示用ウィンドウが保持する状態
*/
struct MenuHost
{
	HWND ownerWindow = nullptr;
	IContextMenu* contextMenu = nullptr;
	IContextMenu2* contextMenu2 = nullptr;

	// 待ち合わせ用: 当プロセスに表示されているウインドウの数と、出現したかどうか
	int visibleWindowCount = 0;
	bool visibleWindowAppeared = false;
};

/**
  可視ウィンドウを数えるための情報
*/
struct VisibleWindowCounter
{
	DWORD processId = 0;
	HWND excludeWindow = nullptr;
	int count = 0;
};

/**
  トップレベルウィンドウを調べるコールバック
  @param[in] hwnd    ウインドウハンドル
  @param[in] lParam  VisibleWindowCounterへのポインタ
  @return TRUE:列挙続行 FALSE:列挙終了
*/
BOOL CALLBACK CountVisibleWindowProc(HWND hwnd, LPARAM lParam)
{
	VisibleWindowCounter* counter = reinterpret_cast<VisibleWindowCounter*>(lParam);
	if (hwnd == counter->excludeWindow) {
		return TRUE;
	}
	// 入力支援ウインドウなど非表示のウインドウは対象外とする
	if (IsWindowVisible(hwnd) == FALSE) {
		return TRUE;
	}
	// シェルが表示するウインドウは当プロセス内に作られるため、
	// 当プロセス以外のウインドウは対象外とする
	DWORD processId = 0;
	GetWindowThreadProcessId(hwnd, &processId);
	if (processId != counter->processId) {
		return TRUE;
	}
	++counter->count;
	return TRUE;
}

/**
  当プロセスに表示されているトップレベルウインドウの数を数える
  シェルが表示するダイアログは当プロセス内に作られるため、
  その出現・消滅を待ち合わせの判定に利用する。
  @param[in] excludeWindow 除外するウインドウ(所有ウインドウ)
  @return 該当するウインドウの数
*/
int CountVisibleProcessWindows(HWND excludeWindow)
{
	VisibleWindowCounter counter{ GetCurrentProcessId(), excludeWindow, 0 };
	EnumWindows(CountVisibleWindowProc, reinterpret_cast<LPARAM>(&counter));
	return counter.count;
}

/**
  実行されたコマンドの完了を待ち合わせる
  シェル側の一部verb(プロパティ等)は非同期に処理されるため、呼び出し元プロセスが
  終了すると処理が取り消される。そのため、処理が完了するまでプロセスを存活させる。
  なお、シェルが表示するダイアログは当プロセスの所有ウインドウを親とした
  モーダルダイアログになるため、待ち合わせ中はメッセージ処理がダイアログ側に
  奪われる。そのため、状態の確認はタイマーで行う。
  @param[in] host        メニューの所有状態
  @param[in] gracePeriod  ウインドウが出現しなかった場合の猶予時間(ミリ秒)
  @param[in] timeout      待ち合わせの上限時間(ミリ秒)
*/
void WaitForCommandCompletion(MenuHost* host, DWORD gracePeriod, DWORD timeout)
{
	const DWORD startTime = GetTickCount();
	SetTimer(host->ownerWindow, COMMAND_TIMER_ID, COMMAND_WAIT_INTERVAL, nullptr);
	for (;;) {
		// 待ち合わせ中もメッセージを処理する
		// (モーダルダイアログが表示されている場合は、この処理がダイアログの
		//  メッセージループ内で行われ、戻ってくるまで処理が中断する)
		MSG message{};
		while (PeekMessage(&message, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			DispatchMessageW(&message);
		}

		if (host->visibleWindowCount > 0) {
			host->visibleWindowAppeared = true;
		}
		else if (host->visibleWindowAppeared) {
			// シェルが表示したウインドウが閉じられたので完了とする
			break;
		}

		const DWORD elapsed = GetTickCount() - startTime;
		if (elapsed >= timeout) {
			break;
		}
		if (host->visibleWindowAppeared == false && elapsed >= gracePeriod) {
			// ウインドウを表示しないverbは猶予時間の経過で終了する
			break;
		}

		Sleep(COMMAND_WAIT_INTERVAL);
	}
	KillTimer(host->ownerWindow, COMMAND_TIMER_ID);
}

/**
  メニューの所有ウィンドウのメッセージプロシージャ
  @param[in] hwnd    ウインドウハンドル
  @param[in] message メッセージ
  @param[in] wParam  メッセージパラメータ
  @param[in] lParam  メッセージパラメータ
  @return メッセージ処理結果
*/
LRESULT CALLBACK OwnerWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_NCCREATE) {
		const CREATESTRUCTW* createStruct = reinterpret_cast<const CREATESTRUCTW*>(lParam);
		SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(createStruct->lpCreateParams));
		return TRUE;
	}

	if (message == WM_TIMER && wParam == COMMAND_TIMER_ID) {
		// 待ち合わせ中の状態を更新する
		// (モーダルダイアログの表示中もタイマーの処理が実行されるため、
		//  状態を確認できる)
		MenuHost* host = reinterpret_cast<MenuHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (host != nullptr) {
			host->visibleWindowCount = CountVisibleProcessWindows(host->ownerWindow);
			if (host->visibleWindowCount > 0) {
				host->visibleWindowAppeared = true;
			}
		}
		return 0;
	}

	if (message == WM_INITMENUPOPUP || message == WM_MENUCHAR ||
		message == WM_DRAWITEM || message == WM_MEASUREITEM) {
		// シェルのコンテキストメニューへメッセージを転送して拡張メニューなどを処理させる
		MenuHost* host = reinterpret_cast<MenuHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
		if (host != nullptr && host->contextMenu2 != nullptr) {
			const HRESULT result = host->contextMenu2->HandleMenuMsg(message, wParam, lParam);
			if (message == WM_MENUCHAR && result == S_OK) {
				return TRUE;
			}
			return 0;
		}
	}

	return DefWindowProcW(hwnd, message, wParam, lParam);
}

/**
  メニューの所有ウィンドウを作成する
  @param[in,out] host 保持する状態
  @return true:作成成功 false:作成失敗
*/
bool CreateOwnerWindow(MenuHost* host)
{
	WNDCLASSEXW windowClass{};
	windowClass.cbSize = sizeof(windowClass);
	windowClass.lpfnWndProc = OwnerWindowProc;
	windowClass.hInstance = GetModuleHandleW(nullptr);
	windowClass.lpszClassName = OWNER_WINDOW_CLASS_NAME;
	if (RegisterClassExW(&windowClass) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
		return false;
	}

	// メニューを所有するための非表示のウインドウを作成する
	host->ownerWindow = CreateWindowExW(
		0,
		OWNER_WINDOW_CLASS_NAME,
		nullptr,
		WS_OVERLAPPED,
		0,
		0,
		0,
		0,
		nullptr,
		nullptr,
		GetModuleHandleW(nullptr),
		host);
	return host->ownerWindow != nullptr;
}

/**
  パスに対応する項目識別子リストを取得する
  @param[in]  path     対象のパス
  @param[out] idList   絶対項目識別子リスト
  @param[out] groupKey 親フォルダごとのグループ分け用のキー
  @return true:取得成功 false:取得失敗
*/
bool CreateItemIdList(const std::wstring& path, PIDLIST_ABSOLUTE& idList, std::wstring& groupKey)
{
	IShellItem* shellItem = nullptr;
	const HRESULT createResult = SHCreateItemFromParsingName(
		path.c_str(),
		nullptr,
		IID_IShellItem,
		reinterpret_cast<void**>(&shellItem));
	if (FAILED(createResult) || shellItem == nullptr) {
		return false;
	}

	// 親フォルダごとにまとめるためのキーを取得する
	IShellItem* parentItem = nullptr;
	if (SUCCEEDED(shellItem->GetParent(&parentItem)) && parentItem != nullptr) {
		LPWSTR parentPath = nullptr;
		if (SUCCEEDED(parentItem->GetDisplayName(SIGDN_FILESYSPATH, &parentPath)) && parentPath != nullptr) {
			groupKey.assign(parentPath);
			CoTaskMemFree(parentPath);
		}
		else {
			// 親フォルダを特定できない場合は単独のグループとして扱う
			groupKey = path;
		}
		parentItem->Release();
	}
	else {
		groupKey = path;
	}

	const HRESULT result = SHGetIDListFromObject(shellItem, &idList);
	shellItem->Release();
	return SUCCEEDED(result);
}

/**
  同じ親フォルダに属する項目のグループ
*/
struct ItemGroup
{
	std::wstring key;
	std::vector<PIDLIST_ABSOLUTE> idLists;
};

/**
  グループに対するコンテキストメニューを表示する
  @param[in] host     メニューの所有ウィンドウ
  @param[in] idLists  対象の項目(絶対項目識別子リスト)
  @param[in] position 表示位置
  @return true:コマンドが選択された false:選択されなかった
*/
bool ShowGroupMenu(MenuHost* host, const std::vector<PIDLIST_ABSOLUTE>& idLists, const POINT& position)
{
	if (idLists.empty()) {
		return false;
	}

	// 親フォルダを取得する
	IShellFolder* parentFolder = nullptr;
	HRESULT result = SHBindToParent(idLists.front(), IID_IShellFolder, reinterpret_cast<void**>(&parentFolder), nullptr);
	if (FAILED(result) || parentFolder == nullptr) {
		return false;
	}

	// 親フォルダ内の項目識別子リストを並べる
	// 絶対項目識別子リストの最後の項目が、親フォルダ内の項目に相当する
	std::vector<PCUITEMID_CHILD> children;
	children.reserve(idLists.size());
	for (PIDLIST_ABSOLUTE idList : idLists) {
		children.push_back(ILFindLastID(idList));
	}

	// コンテキストメニューを取得する
	IContextMenu* contextMenu = nullptr;
	result = parentFolder->GetUIObjectOf(
		host->ownerWindow,
		static_cast<UINT>(children.size()),
		children.data(),
		IID_IContextMenu,
		nullptr,
		reinterpret_cast<void**>(&contextMenu));
	if (FAILED(result) || contextMenu == nullptr) {
		parentFolder->Release();
		return false;
	}

	// メニュー通知を転送するための拡張インターフェースを取得する
	IContextMenu2* contextMenu2 = nullptr;
	contextMenu->QueryInterface(IID_IContextMenu2, reinterpret_cast<void**>(&contextMenu2));

	// メニューを構築する
	HMENU menu = CreatePopupMenu();
	if (menu == nullptr) {
		if (contextMenu2 != nullptr) {
			contextMenu2->Release();
		}
		contextMenu->Release();
		parentFolder->Release();
		return false;
	}

	result = contextMenu->QueryContextMenu(menu, 0, MENU_COMMAND_ID_FIRST, MENU_COMMAND_ID_LAST, CMF_NORMAL);
	if (FAILED(result)) {
		DestroyMenu(menu);
		if (contextMenu2 != nullptr) {
			contextMenu2->Release();
		}
		contextMenu->Release();
		parentFolder->Release();
		return false;
	}

	// メニューを表示して、選択されたコマンドを実行する
	host->contextMenu = contextMenu;
	host->contextMenu2 = contextMenu2;
	const UINT command = TrackPopupMenuEx(
		menu,
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD,
		position.x,
		position.y,
		host->ownerWindow,
		nullptr);
	host->contextMenu = nullptr;
	host->contextMenu2 = nullptr;

	bool invoked = false;
	if (command != 0) {
		// 選択されたコマンドIDからverbのインデックスを算出して実行する
		CMINVOKECOMMANDINFO invokeInfo{};
		invokeInfo.cbSize = sizeof(invokeInfo);
		invokeInfo.fMask = CMIC_MASK_ASYNCOK;
		invokeInfo.hwnd = host->ownerWindow;
		invokeInfo.lpVerb = MAKEINTRESOURCEA(command - MENU_COMMAND_ID_FIRST);
		invokeInfo.nShow = SW_SHOWNORMAL;
		contextMenu->InvokeCommand(&invokeInfo);
		invoked = true;

		// シェル側の処理(ダイアログの表示など)が完了するまで待ち合わせる
		// この待ち合わせ中は menu/contextMenu などを解放しない
		WaitForCommandCompletion(host, COMMAND_WAIT_GRACE_PERIOD, COMMAND_WAIT_TIMEOUT);
	}

	DestroyMenu(menu);
	if (contextMenu2 != nullptr) {
		contextMenu2->Release();
	}
	contextMenu->Release();
	parentFolder->Release();
	return invoked;
}

} // 名前空間

struct ShellContextMenu::PImpl
{
	MenuHost host;
};

ShellContextMenu::ShellContextMenu()
	: in(std::make_unique<PImpl>())
{
}

ShellContextMenu::~ShellContextMenu()
{
	if (in->host.ownerWindow != nullptr) {
		DestroyWindow(in->host.ownerWindow);
		in->host.ownerWindow = nullptr;
	}
}

bool ShellContextMenu::Show(const std::vector<std::wstring>& paths, const POINT& position)
{
	if (paths.empty()) {
		return false;
	}

	// 親フォルダごとにパスをまとめる
	std::vector<ItemGroup> groups;
	for (const std::wstring& path : paths) {
		PIDLIST_ABSOLUTE idList = nullptr;
		std::wstring groupKey;
		if (!CreateItemIdList(path, idList, groupKey)) {
			continue;
		}

		bool grouped = false;
		for (ItemGroup& group : groups) {
			if (CompareStringOrdinal(group.key.c_str(), -1, groupKey.c_str(), -1, TRUE) == CSTR_EQUAL) {
				group.idLists.push_back(idList);
				grouped = true;
				break;
			}
		}
		if (!grouped) {
			ItemGroup group;
			group.key = groupKey;
			group.idLists.push_back(idList);
			groups.push_back(group);
		}
	}

	if (groups.empty()) {
		return false;
	}

	// メニューの所有ウィンドウを作成する
	if (in->host.ownerWindow == nullptr) {
		if (!CreateOwnerWindow(&in->host)) {
			return false;
		}
	}

	// グループごとにメニューを表示する
	bool invoked = false;
	for (const ItemGroup& group : groups) {
		if (ShowGroupMenu(&in->host, group.idLists, position)) {
			invoked = true;
			break;
		}
	}

	// 取得した項目識別子リストを解放する
	for (const ItemGroup& group : groups) {
		for (PIDLIST_ABSOLUTE idList : group.idLists) {
			ILFree(idList);
		}
	}

	return invoked;
}