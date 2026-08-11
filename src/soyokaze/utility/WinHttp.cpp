#include "pch.h"
#include "WinHttp.h"
#include <winhttp.h>
#include "utility/WipingBuffer.h"
#include "utility/WipingString.h"
#include "spdlog/stopwatch.h"

#pragma comment (lib, "winhttp.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {

class WinHttpHandle
{
public:
	WinHttpHandle(HINTERNET h) : mHandle(h) {}
	~WinHttpHandle()
 	{ 
		if (mHandle) {
			WinHttpCloseHandle(mHandle);
		}
	}

	operator HINTERNET() { return mHandle; }

	HINTERNET mHandle;
};


struct WinHttp::PImpl
{
	bool IsContentHTML(const std::vector<WCHAR>& content);
	bool LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML, bool requireHTML);
	bool ConfigureSystemProxy(HINTERNET session, LPCWSTR url);

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

	int mProxyType{SYSTEMSETTING};
	CString mProxyHost;
	CString mProxyUser;
	WipingString mProxyPassword;
	CString mServerUser;
	WipingString mServerPassword;
	CStringW mMethod{L"GET"};
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

bool WinHttp::PImpl::ConfigureSystemProxy(HINTERNET session, LPCWSTR url)
{
	WINHTTP_CURRENT_USER_IE_PROXY_CONFIG ieProxyConfig{};
	if (WinHttpGetIEProxyConfigForCurrentUser(&ieProxyConfig) == FALSE) {
		// システム設定を取得できない場合は、呼び出し元で自動プロキシにフォールバックする
		return false;
	}

	WINHTTP_PROXY_INFO proxyInfo{};
	bool hasProxyInfo = false;

	if (ieProxyConfig.lpszProxy != nullptr) {
		// 手動で指定されたプロキシと、プロキシを使用しない対象を適用する
		proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NAMED_PROXY;
		proxyInfo.lpszProxy = ieProxyConfig.lpszProxy;
		proxyInfo.lpszProxyBypass = ieProxyConfig.lpszProxyBypass;
		hasProxyInfo = true;
	}
	else if (ieProxyConfig.fAutoDetect || ieProxyConfig.lpszAutoConfigUrl != nullptr) {
		// 自動検出またはPACファイルを使って対象URLのプロキシを解決する
		WINHTTP_AUTOPROXY_OPTIONS options{};
		if (ieProxyConfig.fAutoDetect) {
			options.dwFlags |= WINHTTP_AUTOPROXY_AUTO_DETECT;
			options.dwAutoDetectFlags = WINHTTP_AUTO_DETECT_TYPE_DHCP | WINHTTP_AUTO_DETECT_TYPE_DNS_A;
		}
		if (ieProxyConfig.lpszAutoConfigUrl != nullptr) {
			options.dwFlags |= WINHTTP_AUTOPROXY_CONFIG_URL;
			options.lpszAutoConfigUrl = ieProxyConfig.lpszAutoConfigUrl;
		}
		options.fAutoLogonIfChallenged = TRUE;

		hasProxyInfo = WinHttpGetProxyForUrl(session, url, &options, &proxyInfo) != FALSE;
	}
	else {
		// システム設定でプロキシが無効になっている場合は直接接続する
		proxyInfo.dwAccessType = WINHTTP_ACCESS_TYPE_NO_PROXY;
		hasProxyInfo = true;
	}

	bool result = false;
	if (hasProxyInfo) {
		// 解決したプロキシ設定をセッションに反映する
		result = WinHttpSetOption(session, WINHTTP_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo)) != FALSE;
	}

	// WinHTTP APIが確保したプロキシ情報を解放する
	if (proxyInfo.lpszProxy != nullptr && proxyInfo.lpszProxy != ieProxyConfig.lpszProxy) {
		GlobalFree(proxyInfo.lpszProxy);
	}
	if (proxyInfo.lpszProxyBypass != nullptr && proxyInfo.lpszProxyBypass != ieProxyConfig.lpszProxyBypass) {
		GlobalFree(proxyInfo.lpszProxyBypass);
	}
	if (ieProxyConfig.lpszAutoConfigUrl != nullptr) {
		GlobalFree(ieProxyConfig.lpszAutoConfigUrl);
	}
	if (ieProxyConfig.lpszProxy != nullptr) {
		GlobalFree(ieProxyConfig.lpszProxy);
	}
	if (ieProxyConfig.lpszProxyBypass != nullptr) {
		GlobalFree(ieProxyConfig.lpszProxyBypass);
	}

	return result;
}




WinHttp::WinHttp() : in(new PImpl)
{
}

WinHttp::~WinHttp()
{
}

static DWORD ChooseAuthScheme(DWORD supportedSchemes)
{
  if (supportedSchemes & WINHTTP_AUTH_SCHEME_NEGOTIATE) {
    return WINHTTP_AUTH_SCHEME_NEGOTIATE;
	}
  else if (supportedSchemes & WINHTTP_AUTH_SCHEME_NTLM) {
    return WINHTTP_AUTH_SCHEME_NTLM;
	}
  else if (supportedSchemes & WINHTTP_AUTH_SCHEME_PASSPORT) {
    return WINHTTP_AUTH_SCHEME_PASSPORT;
	}
  else if (supportedSchemes & WINHTTP_AUTH_SCHEME_DIGEST) {
    return WINHTTP_AUTH_SCHEME_DIGEST;
	}
	else if (supportedSchemes & WINHTTP_AUTH_SCHEME_BASIC) {
		return WINHTTP_AUTH_SCHEME_BASIC;
	}
  else {
    return 0;
	}
}

bool WinHttp::PImpl::LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML, bool requireHTML)
{
	spdlog::stopwatch sw;

	WinHttpHandle session(WinHttpOpen(L"WinHttpOpen/1.0", GetProxyAccessType(), GetProxyName(), WINHTTP_NO_PROXY_BYPASS, 0));

	WCHAR hostName[1024];
	std::vector<WCHAR> urlPath(65536);
	WCHAR user[256];
	WipingBuffer password(256);

	URL_COMPONENTS cmp={};
	cmp.dwStructSize = sizeof(cmp);
	cmp.lpszHostName = hostName;
	cmp.dwHostNameLength = 1024;
	cmp.lpszUrlPath = urlPath.data();
	cmp.dwUrlPathLength = (int)urlPath.size();
	cmp.lpszUserName = user;
	cmp.dwUserNameLength = 256;
	cmp.lpszPassword = password.Data();
	cmp.dwPasswordLength = (DWORD)password.Length();

	if (WinHttpCrackUrl(url, url.GetLength(), 0, &cmp) == FALSE) {
		spdlog::error(_T("Invalid url. {}"), (LPCTSTR)url);
		return false;
	}

	if (mProxyType == SYSTEMSETTING) {
		// WinHTTPはWinINetのシステム設定をすべて自動では引き継がないため、
		// 現在のユーザーのプロキシ、PAC、バイパス設定を明示的に適用する
		ConfigureSystemProxy(session, url);
	}

	spdlog::debug("httpopen {:.6f} s.", sw);

	WinHttpHandle connect(WinHttpConnect(session, hostName, cmp.nPort, 0));

	spdlog::debug("connect {:.6f} s.", sw);

	WinHttpHandle req(WinHttpOpenRequest(connect, mMethod, urlPath.data(), nullptr, WINHTTP_NO_REFERER,
				WINHTTP_DEFAULT_ACCEPT_TYPES, (INTERNET_SCHEME_HTTPS == cmp.nScheme) ? WINHTTP_FLAG_SECURE : 0));

	spdlog::debug("openrequest {:.6f} s.", sw);

	DWORD proxyScheme = 0;

	
	bool isRetryProxyAuth = (mProxyType == DIRECTPROXY);
		// 指摘したプロキシを使う設定の場合だけ、407が返ったら認証情報をセットしてリトライを試みる

	bool hasRetriedHttpAuth = false;
		// HTTP認証は一度だけリトライする

	DWORD stsCode = 0;

	bool isIncomplete = true;
	while(isIncomplete) {

		BOOL isOK = FALSE;

		// プロキシ認証情報はリトライが複数回行われる可能性があるため、ここでセットする
		if (proxyScheme != 0) {
			isOK = WinHttpSetCredentials(req, WINHTTP_AUTH_TARGET_PROXY, proxyScheme, mProxyUser, mProxyPassword, nullptr);
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

			if (hasRetriedHttpAuth) {
				// リトライ済の場合はあきらめる
				spdlog::error("401 : authentication failed.");
				return false;
			}

			DWORD supporteScheme;
			DWORD firstScheme;
			DWORD target;

			isOK = WinHttpQueryAuthSchemes(req, &supporteScheme, &firstScheme, &target);

			DWORD selectedScheme = ChooseAuthScheme(supporteScheme);

			if (selectedScheme == 0) {
				spdlog::error(_T("401 : unknown scheme {}"), supporteScheme);
				return false;
			}
			// 認証情報を設定
			isOK = WinHttpSetCredentials(req, target, selectedScheme, mServerUser, mServerPassword, nullptr);

			hasRetriedHttpAuth = true;
			continue;
		}
		// プロキシ認証が必要
		else if (stsCode == 407) {

			// 2回目以降はリトライしない(無限リトライになってしまうのを防ぐため)
			if (isRetryProxyAuth == false) {
				spdlog::error(_T("407 : failed to authenticate."));
				return false;
			}

			DWORD supporteScheme;
			DWORD firstScheme;
			DWORD target;

			isOK = WinHttpQueryAuthSchemes(req, &supporteScheme, &firstScheme, &target);

			proxyScheme = ChooseAuthScheme(supporteScheme);
			if (proxyScheme == 0) {
				spdlog::error(_T("407 : unknown scheme {}"), supporteScheme);
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
	if (IsContentHTML(hdrData) == false) {
		// HTMLでなければこのツールでは取り扱わないのでコンテンツを取得せずに抜ける
		isHTML = false;
		if (requireHTML) {
			return false;
		}
	}
	else {
		isHTML = true;
	}

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

bool WinHttp::LoadContent(const CString& url, std::vector<BYTE>& content, bool& isHTML)
{
	return in->LoadContent(url, content, isHTML, true);
}

bool WinHttp::LoadBinaryContent(const CString& url, std::vector<BYTE>& content)
{
	bool isHTML = false;
	return in->LoadContent(url, content, isHTML, false);
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

void WinHttp::SetMethod(LPCWSTR method)
{
	in->mMethod = method;
}

} // end of namespace launcherapp

