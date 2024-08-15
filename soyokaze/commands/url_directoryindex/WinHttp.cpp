#include "pch.h"
#include "WinHttp.h"
#include "commands/url_directoryindex/WinHttpHandle.h"
#include "spdlog/stopwatch.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

struct WinHttp::PImpl
{
	bool IsContentHTML(const std::vector<WCHAR>& content);

	DWORD GetProxyAccessType() {
		if (mProxyType == DIRECTPROXY) { return WINHTTP_ACCESS_TYPE_NAMED_PROXY; }
		else if (mProxyType == NOPROXY) { return WINHTTP_ACCESS_TYPE_NO_PROXY; }
		return WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;
	}

	LPCWSTR GetProxyName() {
		if (mProxyType != DIRECTPROXY) {
			return WINHTTP_NO_PROXY_NAME;
		}
		return mProxyHost;
	}

	int mProxyType = SYSTEMSETTING;
	CString mProxyHost;
	CString mProxyUser;
	CString mProxyPassword;
	CString mServerUser;
	CString mServerPassword;
};

bool WinHttp::PImpl::IsContentHTML(const std::vector<WCHAR>& content)
{
	CString str(content.data());

	int n = 0;
	CString tok = str.Tokenize(_T("\r\n"), n);
	while(tok.IsEmpty() == FALSE) {

		int pos = tok.Find(_T(":"));
		if (pos != -1) {
 			auto key = tok.Left(pos);
			key.Trim();
			auto val = tok.Mid(pos+1);
			val.Trim();

			if (key.CompareNoCase(_T("Content-Type")) == 0) {

				bool isHTML = val.Find(_T("text/html")) != -1;

				if (isHTML == false) {
					spdlog::debug(_T("Content-Type:{}"), (LPCTSTR)val);
				}

				return isHTML;
			}
		}

		tok = str.Tokenize(_T("\r\n"), n);
	}

	spdlog::debug("no Content-Type");
	return false;
}




WinHttp::WinHttp() : in(new PImpl)
{
}

WinHttp::~WinHttp()
{
}

static DWORD ChooseAuthScheme(DWORD supportedSchemes)
{
  if( supportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE ) {
    return WINHTTP_AUTH_SCHEME_NEGOTIATE;
	}
  else if( supportedSchemes & WINHTTP_AUTH_SCHEME_NTLM ) {
    return WINHTTP_AUTH_SCHEME_NTLM;
	}
  else if( supportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT ) {
    return WINHTTP_AUTH_SCHEME_PASSPORT;
	}
  else if( supportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST ) {
    return WINHTTP_AUTH_SCHEME_DIGEST;
	}
  else {
    return 0;
	}
}

bool WinHttp::LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML)
{
	spdlog::stopwatch sw;

 	WinHttpHandle session(WinHttpOpen(L"WinHttpOpen/1.0", in->GetProxyAccessType(), in->GetProxyName(), WINHTTP_NO_PROXY_BYPASS, 0));

	WCHAR hostName[1024];
	std::vector<WCHAR> urlPath(65536);
	WCHAR user[256];
	WCHAR password[256];

	URL_COMPONENTS cmp={};
	cmp.dwStructSize = sizeof(cmp);
	cmp.lpszHostName = hostName;
	cmp.dwHostNameLength = 1024;
	cmp.lpszUrlPath = urlPath.data();
	cmp.dwUrlPathLength = (int)urlPath.size();
	cmp.lpszUserName = user;
	cmp.dwUserNameLength = 256;
	cmp.lpszPassword = password;
	cmp.dwPasswordLength = 256;

	if (WinHttpCrackUrl(url, url.GetLength(), 0, &cmp) == FALSE) {
		return false;
	}

	spdlog::debug("httpopen {:.6f} s.", sw);

	WinHttpHandle connect(WinHttpConnect(session, hostName, cmp.nPort, 0));

	spdlog::debug("connect {:.6f} s.", sw);

	WinHttpHandle req(WinHttpOpenRequest(connect, L"GET", urlPath.data(), nullptr, WINHTTP_NO_REFERER, 
				WINHTTP_DEFAULT_ACCEPT_TYPES, (INTERNET_SCHEME_HTTPS == cmp.nScheme) ? WINHTTP_FLAG_SECURE : 0));

	//LPCWSTR headers = L"Accept: */*\r\nPragma: no-cache\r\nCache-Control: no-cache\r\n";

	spdlog::debug("openrequest {:.6f} s.", sw);

	DWORD proxyScheme = 0;

	
	bool isRetryProxyAuth = (in->mProxyType == DIRECTPROXY);
		// 指摘したプロキシを使う設定の場合だけ、407が返ったら認証情報をセットしてリトライを試みる

	DWORD stsCode = 0;

	bool isIncomplete = true;
	while(isIncomplete) {

		BOOL isOK = FALSE;

		// プロキシ認証情報はリトライが複数回行われる可能性があるため、ここでセットする
		if (proxyScheme != 0) {
			isOK = WinHttpSetCredentials(req, WINHTTP_AUTH_TARGET_PROXY, proxyScheme, in->mProxyUser, in->mProxyPassword, nullptr);
		}

		// リクエストを出す
		isOK = WinHttpSendRequest(req, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, WINHTTP_IGNORE_REQUEST_TOTAL_LENGTH, 0);
		if (isOK == FALSE) {
			spdlog::debug(_T("Failed to WinHttpSendRequest"));
			return false;
		}

		spdlog::debug("sendrequest {:.6f} s.", sw);

		// レスポンスをうけとる
		if (WinHttpReceiveResponse(req, NULL) == FALSE) {

			if (GetLastError() == ERROR_WINHTTP_RESEND_REQUEST) {
				// リトライ
				continue;
			}

			spdlog::debug(_T("Failed to WinHttpReceiveResponse"));
			return false;
		}

		spdlog::debug("receiveresponse {:.6f} s.", sw);

		// ステータスコードを得る
		DWORD hdrSize = sizeof(stsCode);
		WinHttpQueryHeaders(req, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &stsCode, &hdrSize, WINHTTP_NO_HEADER_INDEX);

		// サーバ認証が必要
		if (stsCode == 401) {
			DWORD supporteScheme;
			DWORD firstScheme;
			DWORD target;

			isOK = WinHttpQueryAuthSchemes(req, &supporteScheme, &firstScheme, &target);

			DWORD selectedScheme = ChooseAuthScheme(supporteScheme);

			if (selectedScheme == 0) {
				return false;
			}
			// 認証情報を設定
			isOK = WinHttpSetCredentials(req, target, selectedScheme, in->mServerUser, in->mServerPassword, nullptr);
			continue;
		}
		// プロキシ認証が必要
		else if (stsCode == 407) {

			// 2回目以降はリトライしない(無限リトライになってしまうのを防ぐため)
			if (isRetryProxyAuth == false) {
				return false;
			}

			DWORD supporteScheme;
			DWORD firstScheme;
			DWORD target;

			isOK = WinHttpQueryAuthSchemes(req, &supporteScheme, &firstScheme, &target);

			proxyScheme = ChooseAuthScheme(supporteScheme);
			if (proxyScheme == 0) {
				return false;
			}
			isRetryProxyAuth = false;
			continue;
		}

		if (stsCode != HTTP_STATUS_OK) {
			spdlog::debug(_T("status is not HTTP_STATUS_OK"));
			return false;
		}

		break;
	}

	// データのサイズを得る
	DWORD hdrSize = 0;
	BOOL isOK = WinHttpQueryHeaders(req, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, WINHTTP_NO_OUTPUT_BUFFER, &hdrSize, WINHTTP_NO_HEADER_INDEX);

	// データを取得する
	std::vector<WCHAR> hdrData(hdrSize);
	isOK = WinHttpQueryHeaders(req, WINHTTP_QUERY_RAW_HEADERS_CRLF, WINHTTP_HEADER_NAME_BY_INDEX, hdrData.data(), &hdrSize, WINHTTP_NO_HEADER_INDEX);
	if (isOK == FALSE) {
		spdlog::debug(_T("Failed to WinHttpQueryHeaders"));
		return false;
	}

	// ヘッダをパースしてコンテンツ種別を得る
	if (in->IsContentHTML(hdrData) == false) {
		// HTMLでなければこのツールでは取り扱わないのでコンテンツを取得せずに抜ける
		isHTML = false;
		return false;
	}
	isHTML = true;

	spdlog::debug("queryheaders {:.6f} s.", sw);

	std::vector<BYTE> buff;

	size_t offset = 0;
	for(;;) {
		DWORD availableData = 0;
		if (WinHttpQueryDataAvailable(req, &availableData) == FALSE) {
			spdlog::debug(_T("WinHttpQueryDataAvailable"));
			break;
		}
		if (availableData == 0) {
			break;
		}
		buff.resize(buff.size() + availableData);

		if (WinHttpReadData(req, buff.data() + offset, availableData, nullptr) == FALSE) {
			spdlog::debug(_T("WinHttpReadData"));
			break;
		}

		offset += availableData;
	}

	content.swap(buff);

	spdlog::debug(_T("URL : {}"), (LPCTSTR)url);
	spdlog::debug("download {:.6f} s.", sw);

	return true;
}

void WinHttp::SetProxyType(int type)
{
	in->mProxyType= type;
}
void WinHttp::SetProxyCredential(const CString& host, const CString& user, const CString& password)
{
	in->mProxyHost = host;
	in->mProxyUser = user;
	in->mProxyPassword = password;
}

void WinHttp::SetServerCredential(const CString& user, const CString& password)
{
	in->mServerUser = user;
	in->mServerPassword = password;
}

} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

