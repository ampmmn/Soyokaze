#include "pch.h"
#include "MainWindowHotKey.h"
#include "setting/AppPreference.h"
#include "hotkey/CommandHotKeyManager.h"
#include "hotkey/CommandHotKeyHandlerIF.h"
#include "mainwindow/controller/MainWindowController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


class MainWindowHotKey::SelectUpHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~SelectUpHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_up"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->InputKey(VK_UP);
		return true;
	}
};


class MainWindowHotKey::SelectDownHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~SelectDownHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_down"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->InputKey(VK_DOWN);
		return true;
	}
};

class MainWindowHotKey::EnterHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~EnterHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_enter"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->InputKey(VK_RETURN);
		return true;
	}
};

class MainWindowHotKey::ComplHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~ComplHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_compl"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->InputKey(VK_TAB);
		return true;
	}
};

class MainWindowHotKey::DeleteWordHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~DeleteWordHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_deleteword"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->DeleteWord();
		return true;
	}
};

class MainWindowHotKey::ContextMenuHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~ContextMenuHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_contextmenu"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->ShowContextMenu();
		return true;
	}
};

class MainWindowHotKey::CopyHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~CopyHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_copy"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->CopyInputText();
		mainWnd->HideWindow();
		return true;
	}
};

class MainWindowHotKey::MoveUpHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~MoveUpHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_moveup"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->MoveTemporary(VK_UP);
		return true;
	}
};

class MainWindowHotKey::MoveDownHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~MoveDownHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_movedown"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->MoveTemporary(VK_DOWN);
		return true;
	}
};

class MainWindowHotKey::MoveLeftHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~MoveLeftHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_moveleft"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->MoveTemporary(VK_LEFT);
		return true;
	}
};

class MainWindowHotKey::MoveRightHandler : public launcherapp::core::CommandHotKeyHandler
{
public:
	virtual ~MoveRightHandler() {}
	CString GetDisplayName() override { return _T("__mainwindow_moveright"); }
	bool Invoke() override {
		auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
		mainWnd->MoveTemporary(VK_RIGHT);
		return true;
	}
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



using CommandHotKeyManager = launcherapp::core::CommandHotKeyManager;

struct MainWindowHotKey::PImpl
{
};

// 修飾キー
// MOD_ALT     (0x0001)
// MOD_CONTROL (0x0002)
// MOD_SHIFT   (0x0004)
// MOD_WIN     (0x0008)
// MOD_NOREPEAT(0x4000)

MainWindowHotKey::MainWindowHotKey() : in(new PImpl)
{
	AppPreference::Get()->RegisterListener(this);
}

MainWindowHotKey::~MainWindowHotKey()
{
	AppPreference::Get()->UnregisterListener(this);
}

// 設定ファイルから設定値を取得してホットキー登録
bool MainWindowHotKey::Register()
{
	SPDLOG_DEBUG(_T("start"));

	auto manager = CommandHotKeyManager::GetInstance();
	auto pref = AppPreference::Get();
	auto settingsPtr = (Settings*)&pref->GetSettings();

	using HotKeyAttr = CommandHotKeyAttribute;

	auto hotKeyAttrUp = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:Up-Modifiers"), 0),
	                                settingsPtr->Get(_T("MainWindowKey:Up-VirtualKeyCode"), -1));
	if (hotKeyAttrUp.GetVKCode() != -1) {
		manager->Register(this, new SelectUpHandler, hotKeyAttrUp);
	}

	auto hotKeyAttrDown = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:Down-Modifiers"), 0),
	                                 settingsPtr->Get(_T("MainWindowKey:Down-VirtualKeyCode"), -1));
	if (hotKeyAttrDown.GetVKCode() != -1) {
		manager->Register(this, new SelectDownHandler, hotKeyAttrDown);
	}

	auto hotKeyAttrEnter = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:Enter-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Enter-VirtualKeyCode"), -1));
	if (hotKeyAttrEnter.GetVKCode() != -1) {
		manager->Register(this, new EnterHandler, hotKeyAttrEnter);
	}

	auto hotKeyAttrCompl = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:Compl-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Compl-VirtualKeyCode"), -1));
	if (hotKeyAttrCompl.GetVKCode() != -1) {
		manager->Register(this, new ComplHandler, hotKeyAttrCompl);
	}

	auto hotKeyAttrDeleteWord = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:DeleteWord-Modifiers"), 0),
	                                       settingsPtr->Get(_T("MainWindowKey:DeleteWord-VirtualKeyCode"), -1));
	if (hotKeyAttrDeleteWord.GetVKCode() != -1) {
		manager->Register(this, new DeleteWordHandler, hotKeyAttrDeleteWord);
	}

	auto hotKeyAttrContextMenu = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:ContextMenu-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:ContextMenu-VirtualKeyCode"), -1));
	if (hotKeyAttrContextMenu.GetVKCode() != -1) {
		manager->Register(this, new ContextMenuHandler, hotKeyAttrContextMenu);
	}

	auto hotKeyAttrCopy = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:Copy-Modifiers"), 0),
	                            settingsPtr->Get(_T("MainWindowKey:Copy-VirtualKeyCode"), -1));
	if (hotKeyAttrCopy.GetVKCode() != -1) {
		manager->Register(this, new CopyHandler, hotKeyAttrCopy);
	}

	auto hotKeyAttrMoveUp = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:MoveUp-Modifiers"), 0),
	                                settingsPtr->Get(_T("MainWindowKey:MoveUp-VirtualKeyCode"), -1));
	if (hotKeyAttrMoveUp.GetVKCode() != -1) {
		manager->Register(this, new MoveUpHandler, hotKeyAttrMoveUp);
	}

	auto hotKeyAttrMoveDown = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:MoveDown-Modifiers"), 0),
	                                settingsPtr->Get(_T("MainWindowKey:MoveDown-VirtualKeyCode"), -1));
	if (hotKeyAttrMoveDown.GetVKCode() != -1) {
		manager->Register(this, new MoveDownHandler, hotKeyAttrMoveDown);
	}

	auto hotKeyAttrMoveLeft = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:MoveLeft-Modifiers"), 0),
	                                settingsPtr->Get(_T("MainWindowKey:MoveLeft-VirtualKeyCode"), -1));
	if (hotKeyAttrMoveLeft.GetVKCode() != -1) {
		manager->Register(this, new MoveLeftHandler, hotKeyAttrMoveLeft);
	}

	auto hotKeyAttrMoveRight = HotKeyAttr(settingsPtr->Get(_T("MainWindowKey:MoveRight-Modifiers"), 0),
	                                settingsPtr->Get(_T("MainWindowKey:MoveRight-VirtualKeyCode"), -1));
	if (hotKeyAttrMoveRight.GetVKCode() != -1) {
		manager->Register(this, new MoveRightHandler, hotKeyAttrMoveRight);
	}

	return true;
}

// 登録解除する
void MainWindowHotKey::Unregister()
{
	auto manager = CommandHotKeyManager::GetInstance();
	manager->Clear(this);
}

// 再登録(登録解除→登録)
bool MainWindowHotKey::Reload()
{
	Unregister();
	return Register();
}

void MainWindowHotKey::OnAppFirstBoot()
{
}

void MainWindowHotKey::OnAppNormalBoot()
{
}

void MainWindowHotKey::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	Reload();
}

void MainWindowHotKey::OnAppExit()
{
	AppPreference::Get()->UnregisterListener(this);
}



