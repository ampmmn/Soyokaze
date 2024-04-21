#include "pch.h"
#include "WarnWorkTimeToast.h"
#include "app/LauncherApp.h"
#include "app/AppName.h"
#include "settingwindow/ShortcutSettingPage.h"

#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <notificationactivationcallback.h>

using namespace winrt;
using namespace winrt::Windows::UI::Notifications;
using namespace winrt::Windows::Data::Xml::Dom;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace mainwindow {

struct Toast::PImpl
{
	int mTh;
};

Toast::Toast() : in(new PImpl)
{
}

Toast::~Toast()
{
}

void Toast::SetThreshold(int th)
{
	in->mTh = th;
}

void Toast::Show()
{
	if (ShortcutSettingPage::IsStartMenuExists() == false) {
		// スタートメニューにショートカットが登録されていない場合はShell_NotifyIconのメッセージで代替する
		CString notifyMsg;
		notifyMsg.Format(_T("%d分以上の連続稼働を検知\n健康のため、適宜休憩を入れるが吉"), in->mTh);
		auto app = (LauncherApp*)AfxGetApp();
		app->PopupMessage(notifyMsg);
		return;
	}

	// Construct the XML toast template
	XmlDocument doc;
	doc.LoadXml(L"\
			<toast>\
			<visual>\
			<binding template=\"ToastGeneric\">\
			<text></text>\
			<text></text>\
			</binding>\
			</visual>\
			</toast>");

	doc.DocumentElement().SetAttribute(L"launch", L"action=warnWorkTime");

	CString buf;
	buf.Format(_T("%d分以上の連続稼働を検知"), in->mTh);

	doc.SelectSingleNode(L"//text[1]").InnerText((LPCTSTR)buf);
	doc.SelectSingleNode(L"//text[2]").InnerText(_T("健康のため、適宜休憩を入れるが吉"));

	winrt::Windows::UI::Notifications::ToastNotification notif(doc);
	winrt::Windows::UI::Notifications::ToastNotificationManager toastManager{};
	ToastNotifier toastNotifier(toastManager.CreateToastNotifier(LAUNCHER_APPID));
	notif.Group(L"healthnotify");
	toastNotifier.Show(notif);

	// FIXME: クリックしたときの通知が、commands/watchpath/WatchPathToastクラス内で定義しているcallback側に飛ぶ。
	//        今後必要に応じて処理を共通化すること
}

void Toast::Clear()
{
	// このクラスによって生成したトースト通知を消す
	try {
		winrt::Windows::UI::Notifications::ToastNotificationManager toastManager{};
		toastManager.History().RemoveGroup(L"healthnotify", LAUNCHER_APPID);
	}
	catch (...) {
	}
}

}
}
