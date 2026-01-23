#include "pch.h"
#include "Manual.h"
#include "utility/Path.h"
#include "control/webbrowser/InternalBrowser.h"
#include "SharedHwnd.h"
#include "resource.h"
#include <map>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using InternalBrowser = soyokaze::control::webbrowser::InternalBrowser;

namespace launcherapp {
namespace app {

struct Manual::PImpl
{
	std::map<String, CString> mPageMapping;
	std::unique_ptr<InternalBrowser> mDocWindow;
};


Manual::Manual() : in(new PImpl)
{
}

Manual::~Manual()
{
}

Manual* Manual::GetInstance()
{
	static Manual inst;
	return &inst;
}

bool Manual::Navigate(const char* pageId)
{
	SPDLOG_DEBUG("start");

	// ヘルプの有無を確認
	Path filePath(Path::MODULEFILEDIR);
	filePath.Append(L"files\\manuals\\index.html");
	if (filePath.FileExists() == FALSE) {
		CString msg((LPCTSTR)IDS_ERR_HELPDOESNOTEXIST);
		msg += _T("\n");
		msg += filePath;
		AfxMessageBox(msg);

		SPDLOG_WARN(L"help file does not exist. filePath:{}", (LPCWSTR)filePath);
		return false;
	}

	// クッションページ?のパス生成
	Path dirPath(Path::MODULEFILEDIR);
	dirPath.Append(L"files\\fragment");
	SPDLOG_DEBUG(L"dirPath:{}", (LPCWSTR)dirPath);

	// 指定されたIDに対応するページの有無を確認
	CString pagePath;
	auto it = in->mPageMapping.find(pageId);
	if (it != in->mPageMapping.end()) {
		pagePath = it->second;
	}
	else {
		UTF2UTF(pageId, pagePath);
	}

	CStringW uri;
	uri.Format(L"file:///%s/%s.html", (LPCWSTR)dirPath, (LPCWSTR)pagePath);
	uri.Replace(L'\\', L'/');

	if (in->mDocWindow.get() == nullptr || in->mDocWindow->GetSafeHwnd() == nullptr) {
		CRect rect(0, 0, 800, 600);
		in->mDocWindow.reset(new InternalBrowser());
		int style = WS_VISIBLE | WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_THICKFRAME;
		in->mDocWindow->Create(nullptr, style, rect, 0);
	}

	in->mDocWindow->Open(uri);
	return true;
}

}
}
