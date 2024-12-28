#include "pch.h"
#include "URLDirectoryIndexCommandParam.h"
#include <winhttp.h>
#include <array>

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

CommandParam::CommandParam() : mProxyType(0)
{
}

CommandParam::CommandParam(const CommandParam& rhs) : 
	mName(rhs.mName),
	mDescription(rhs.mDescription),
	mURL(rhs.mURL),
	mHotKeyAttr(rhs.mHotKeyAttr),
	mServerUser(rhs.mServerUser),
	mServerPassword(rhs.mServerPassword),
	mProxyType(rhs.mProxyType),
	mProxyHost(rhs.mProxyHost),
	mProxyUser(rhs.mProxyUser),
	mProxyPassword(rhs.mProxyPassword)
{
}

CommandParam::~CommandParam()
{
}

CommandParam& CommandParam::operator = (const CommandParam& rhs)
{
	if (this != &rhs) {
		mName = rhs.mName;
		mDescription = rhs.mDescription;
		mURL = rhs.mURL;
		mHotKeyAttr = rhs.mHotKeyAttr;
		mServerUser = rhs.mServerUser;
		mServerPassword = rhs.mServerPassword;
		mProxyType = rhs.mProxyType;
		mProxyHost = rhs.mProxyHost;
		mProxyUser = rhs.mProxyUser;
		mProxyPassword = rhs.mProxyPassword;
	}
	return *this;

}

// サブパスを連結
CString CommandParam::CombineURL(const CString& subPath) const
{
	return CombineURL2(mURL, subPath);
}

// サブパスを連結
CString CommandParam::CombineURL(const CString& subPath1, const CString& subPath2) const
{
	auto url = CombineURL2(mURL, subPath1);
	return CombineURL2(url, subPath2);
}

CString CommandParam::CombineURL2(const CString& urlPart, const CString& subPath)
{
	CString url(urlPart);

	// 絶対表記の場合はそのまま返す
	static tregex regURL(_T("https?://.+"));
	if (std::regex_search((LPCTSTR)subPath, regURL)) {
		url = subPath;
		SimplifyURL(url);

		spdlog::debug(_T("route1 {}"), (LPCTSTR)url);
		return url;
	}

	WCHAR hostName[256];
	std::vector<WCHAR> urlPath(8000);

	URL_COMPONENTS cmp={};
	cmp.dwStructSize = sizeof(cmp);
	cmp.lpszHostName = hostName;
	cmp.dwHostNameLength = 256;
	cmp.lpszUrlPath = urlPath.data();
	cmp.dwUrlPathLength = (int)urlPath.size();
	WinHttpCrackUrl(url, url.GetLength(), 0, &cmp);

	if (subPath.IsEmpty() == FALSE && subPath[0] == _T('/')) {
		// subPathが"/"で始まる場合は、ルートからのパスにする

		_tcsncpy_s(cmp.lpszUrlPath, 8000, subPath, _TRUNCATE);
		cmp.dwUrlPathLength = subPath.GetLength();

		DWORD len =8000;
		WinHttpCreateUrl(&cmp, 0, url.GetBuffer(8000), &len);
		url.ReleaseBuffer();
		SimplifyURL(url);
		return url;
	}

	// urlとsubPathを連結する
	int pos = -1;
	for (int i = url.GetLength()-1;i >= 0; --i) {
		//最後に登場する'/'の位置を探す
		if (url[i] != _T('/')) {
			continue;
		}
		pos = i;
		break;
	}

	if (pos == -1) {
		spdlog::debug(_T("bug"));
		return url;
	}

	// urlの先頭から'/'までを取り出し、subPathを連結する
	url = url.Left(pos + 1);
	url += subPath;

	SimplifyURL(url);
	return url;
}

// URLに含まれる ../ を除去する
void CommandParam::SimplifyURL(CString& url)
{
	tstring wholeText((LPCTSTR)url);

	// まずホスト名までとパスにばらす
	static tregex reg(_T("(https?://[^/]+)(/.*)$"));
	if (std::regex_match(wholeText, reg) == false) {
		return;
	}
	tstring host = std::regex_replace(wholeText, reg, _T("$1"));
	tstring path = std::regex_replace(wholeText, reg, _T("$2"));

	// パス部分に含まれる../を除去する
	static tregex reg2(_T("^(.*?)(/[^/]+)?/\\.\\.(/.+)$"));
	while (std::regex_match(path, reg2)) {
		path = std::regex_replace(path, reg2, _T("$1$3"));
	}

	url = (host + path).c_str();
}

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

