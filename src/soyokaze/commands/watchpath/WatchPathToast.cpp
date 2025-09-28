#include "pch.h"
#include "WatchPathToast.h"
#include "app/AppName.h"
#include "commands/common/SubProcess.h"
#include "actions/core/ActionParameter.h"
#include "utility/Path.h"

#include <winrt/Windows.UI.Notifications.h>
#include <winrt/Windows.Data.Xml.Dom.h>
#include <notificationactivationcallback.h>

using namespace winrt;
using namespace winrt::Windows::UI::Notifications;
using namespace winrt::Windows::Data::Xml::Dom;
using ParameterBuilder = launcherapp::actions::core::ParameterBuilder;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using SubProcess = launcherapp::commands::common::SubProcess;

namespace launcherapp {
namespace commands {
namespace watchpath {

struct callback : winrt::implements<callback, INotificationActivationCallback>
{
	HRESULT __stdcall Activate(
			LPCWSTR app,
			LPCWSTR args,
			[[maybe_unused]] NOTIFICATION_USER_INPUT_DATA const* data,
			[[maybe_unused]] ULONG count) noexcept final
	{
		UNREFERENCED_PARAMETER(app);

		try
		{
			std::map<CString, CString> argsMap;
			parseArguments(args, argsMap);
			auto action = argsMap[_T("action")];
			spdlog::debug(_T("action:{}"), (LPCTSTR)action);
			if (action != _T("showFolder")) {
					return S_OK;
			}

			// フォルダ
			CString path = argsMap[_T("path")];
			if (Path::FileExists(path) == FALSE) {
				return S_OK;
			}
			if (Path::IsDirectory(path) == FALSE) {
				PathRemoveFileSpec(path.GetBuffer(MAX_PATH_NTFS));
				path.ReleaseBuffer();
			}
			if (path.Right(1) != _T('\\')) {
				path += _T("\\");
			}

			// フォルダを開く
			SubProcess exec(ParameterBuilder::EmptyParam());
			SubProcess::ProcessPtr process;
			exec.Run(path, _T(""), process);

			return S_OK;
		}
		catch (...)
		{
			return winrt::to_hresult();
		}
	}

	void parseArguments(LPCWSTR args, std::map<CString, CString>& param)
	{
		CStringW _argstr(args);
		CString argStr(_argstr);

		int n = 0;
		auto tok = argStr.Tokenize(_T("&"), n);
		while(tok.IsEmpty() == FALSE) {
			int sep = tok.Find(_T("="));
			if (sep != -1) {
				auto key = tok.Left(sep);
				auto val = tok.Mid(sep + 1);
				param[key] = val;
			}
			tok = argStr.Tokenize(_T("&"), n);
		}
	}
};

struct callback_factory : implements<callback_factory, IClassFactory>
{
	HRESULT __stdcall CreateInstance(
			IUnknown* outer,
			GUID const& iid,
			void** result) noexcept final
	{
		*result = nullptr;

		if (outer) {
			return CLASS_E_NOAGGREGATION;
		}
		return make<callback>()->QueryInterface(iid, result);
	}

	HRESULT __stdcall LockServer(BOOL) noexcept final
	{
		return S_OK;
	}
};

struct Toast::PImpl
{
	CString mName;
	CString mPath;
	CString mMessage;
	CString mDetail;
};

static void registerCallback()
{
	DWORD registration;
	HRESULT hr = CoRegisterClassObject(LAUNCHER_TOAST_CALLBACK_GUID, make<callback_factory>().get(),
	                      CLSCTX_LOCAL_SERVER, REGCLS_SINGLEUSE, &registration);
	if (FAILED(hr)) {
		spdlog::error(_T("Failed to CoRegisterClassObject!"));
	}
}

static bool isRegistered = false;

Toast::Toast() : in(new PImpl)
{
	in->mMessage = _T("更新を検知");
	// コールバックを登録
	registerCallback();
}

Toast::~Toast()
{
}

void Toast::SetCommandName(const CString& name)
{
	in->mName = name;
}

void Toast::SetPath(const CString& path)
{
	in->mPath = path;
}

void Toast::SetMessage(const CString& message)
{
	in->mMessage = message;
}

void Toast::SetDetail(const CString& detail)
{
	in->mDetail = detail;
}


void Toast::Show()
{
	// Construct the XML toast template
	XmlDocument doc;
	doc.LoadXml(L"\
			<toast>\
			<visual>\
			<binding template=\"ToastGeneric\">\
			<text></text>\
			<text></text>\
			<text></text>\
			</binding>\
			</visual>\
			</toast>");

	CStringW actionStr;
	actionStr.Format(L"action=showFolder&cmdName=%s&path=%s", (LPCWSTR)in->mName, (LPCWSTR)in->mPath);
	doc.DocumentElement().SetAttribute(L"launch", (LPCWSTR)actionStr);

	CString buf;
	buf.Format(_T("[%s] %s"), (LPCTSTR)in->mName, (LPCTSTR)in->mMessage);

	doc.SelectSingleNode(L"//text[1]").InnerText((LPCTSTR)buf);

	doc.SelectSingleNode(L"//text[2]").InnerText((LPCTSTR)in->mDetail);
	doc.SelectSingleNode(L"//text[3]").InnerText(L"クリックするとフォルダを開きます");

	winrt::Windows::UI::Notifications::ToastNotification notif(doc);
	winrt::Windows::UI::Notifications::ToastNotificationManager toastManager{};
	ToastNotifier toastNotifier(toastManager.CreateToastNotifier(LAUNCHER_APPID));
	//notif.Group(L"grp");
	toastNotifier.Show(notif);

	// 特定の通知を消す
	//try {
	//	toastManager.History().RemoveGroup(L"grp", LAUNCHER_APPID);
	//}
	//catch (...) {
	//		int a = 0;
	//	}
}

}
}
}
