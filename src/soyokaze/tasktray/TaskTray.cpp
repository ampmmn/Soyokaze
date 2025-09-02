// あ
#include "pch.h"
#include "framework.h"
#include "TaskTray.h"
#include "icon/IconLoader.h"
#include "app/AppName.h"
#include "hotkey/HotKeyAttribute.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"
#include "utility/Path.h"
#include "utility/VersionInfo.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

const int ID_LAUNCHER_TASKTRAY = 1000;

IMPLEMENT_DYNAMIC(TaskTray, CWnd)

struct TaskTray::PImpl : public AppPreferenceListenerIF
{
// AppPreferenceListenerIF
	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override {
		// 設定が更新されたらタスクトレイのツールチップ文言を更新
		UpdateTips();
	}
	void OnAppExit() override {
	}

	CString MakeTipsText();
	void UpdateTips();

	// タスクトレイウインドウ
	HWND mTaskTrayWindow {nullptr};
	// タスクトレイ関連イベント通知先
	TaskTrayEventListenerIF* mListenerPtr{nullptr};

	// タスクトレイに登録用の情報
	NOTIFYICONDATA mNotifyIconData{};
};

CString TaskTray::PImpl::MakeTipsText()
{
	Path modulePath(Path::MODULEFILEPATH);

	CString tipsStr{APPNAME};

	CString versionStr;
	VersionInfo::GetVersionInfo(versionStr);

	tipsStr += _T(" ");
	tipsStr += versionStr;
	tipsStr += _T("\n");
	tipsStr += _T("表示:[");

	auto pref= AppPreference::Get();
	UINT mod = pref->GetModifiers();
	UINT vk = pref->GetVirtualKeyCode();
	tipsStr += HOTKEY_ATTR(mod, vk).ToString();
	tipsStr += _T("]");

	return tipsStr;
}

void TaskTray::PImpl::UpdateTips()
{
	if (IsWindow(mTaskTrayWindow) == FALSE) {
		return ;
	}

	// タスクトレイテキストの更新
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof(NOTIFYICONDATA);
	nid.hWnd             = mTaskTrayWindow;
	nid.uID              = ID_LAUNCHER_TASKTRAY;
	nid.uFlags           = NIF_TIP;

	auto tipsStr = MakeTipsText();
	_tcsncpy_s(nid.szTip, NELEMENTS(nid.szTip), tipsStr, _TRUNCATE);

	Shell_NotifyIcon(NIM_MODIFY, &nid);
}

TaskTray::TaskTray(TaskTrayEventListenerIF* listener) : in(new PImpl)
{
	in->mListenerPtr = listener;
	memset(&in->mNotifyIconData, 0, sizeof(in->mNotifyIconData));
}

TaskTray::~TaskTray()
{
	AppPreference::Get()->UnregisterListener(in.get());
	Shell_NotifyIcon(NIM_DELETE, &in->mNotifyIconData);
}


BEGIN_MESSAGE_MAP(TaskTray, CWnd)
	ON_MESSAGE(WM_APP+1, OnNotifyTrakTray)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

BOOL TaskTray::Create()
{
	BOOL isOK = CreateEx(WS_EX_TOOLWINDOW, AfxRegisterWndClass(0),
	                     _T("LauncherTaskTray"), WS_OVERLAPPED,
	                     0, 0, 0, 0, NULL, NULL );

	if (isOK == FALSE) {
		return FALSE;
	}

	AppPreference::Get()->RegisterListener(in.get());

	in->mTaskTrayWindow = GetSafeHwnd();

	// タスクトレイ登録
	NOTIFYICONDATA nid;
	nid.cbSize           = sizeof(NOTIFYICONDATA);
	nid.hWnd             = GetSafeHwnd();
	nid.uID              = ID_LAUNCHER_TASKTRAY;
	nid.uCallbackMessage = WM_APP+1;
	nid.uFlags           = NIF_MESSAGE;

	nid.uFlags |= NIF_ICON;
	nid.hIcon   = IconLoader::Get()->LoadTasktrayIcon();

	auto tipsStr = in->MakeTipsText();

	nid.uFlags |= NIF_TIP;
	_tcsncpy_s(nid.szTip, NELEMENTS(nid.szTip), tipsStr, _TRUNCATE);

	in->mNotifyIconData = nid;

	return Shell_NotifyIcon(NIM_ADD, &in->mNotifyIconData);

}

void TaskTray::ShowMessage(const wchar_t* msg, const wchar_t* title)
{
	NOTIFYICONDATAW nid = in->mNotifyIconData;

	nid.cbSize = NOTIFYICONDATA_V3_SIZE;
	nid.uFlags |= NIF_INFO;
	wcsncpy_s(nid.szInfoTitle, title, _TRUNCATE);
	wcsncpy_s(nid.szInfo, msg, _TRUNCATE);
	nid.uFlags |= NIF_ICON;
	nid.hIcon  = IconLoader::Get()->LoadTasktrayIcon();
	nid.dwInfoFlags = NIIF_INFO;
	// Note: 任意のアイコンを表示する場合は下記
	// nid.dwInfoFlags = NIIF_USER;
	// nid.hBalloonIcon = IconLoader::Get()->LoadTasktrayIcon();   // ここで任意のアイコン

	Shell_NotifyIconW(NIM_MODIFY, &nid);
}

void TaskTray::ShowMessage(const char* msg, const char* title)
{
	std::wstring tmp;

	NOTIFYICONDATAW nid = in->mNotifyIconData;

	nid.cbSize = NOTIFYICONDATA_V3_SIZE;
	nid.uFlags |= NIF_INFO;

	UTF2UTF(title, tmp);
	wcsncpy_s(nid.szInfoTitle, tmp.c_str(), _TRUNCATE);
	UTF2UTF(msg, tmp);
	wcsncpy_s(nid.szInfo, tmp.c_str(), _TRUNCATE);
	nid.uFlags |= NIF_ICON;
	nid.hIcon  = IconLoader::Get()->LoadTasktrayIcon();
	nid.dwInfoFlags = NIIF_INFO;

	// Note: 任意のアイコンを表示する場合は下記
	// nid.dwInfoFlags = NIIF_USER;
	// nid.hBalloonIcon = IconLoader::Get()->LoadTasktrayIcon();   // ここで任意のアイコン

	Shell_NotifyIconW(NIM_MODIFY, &nid);
}


LRESULT TaskTray::OnNotifyTrakTray(WPARAM wp, LPARAM lp)
{
	UINT nID   = (UINT)wp;
	UINT msg = (UINT)LOWORD(lp);
	
	if (nID != ID_LAUNCHER_TASKTRAY) {
		return 0;
	}

	if (msg == WM_LBUTTONDBLCLK) {
		in->mListenerPtr->OnTaskTrayLButtonDblclk();
	}
	else if (msg == WM_RBUTTONDOWN) {
		OnContextMenu();
	}

	return 0;
}

void TaskTray::OnContextMenu()
{
	CPoint pos;
	GetCursorPos(&pos);
	in->mListenerPtr->OnTaskTrayContextMenu(this, pos);
}

