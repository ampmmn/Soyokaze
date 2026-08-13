#include "pch.h"
#include "FaviconLoader.h"
#include <utility/WinHttp.h>
#include <WinHttp.h>
#include <atlimage.h>
#include <shlwapi.h>
#include <objidl.h>
#include <limits>
#include <regex>

#pragma comment(lib, "shlwapi.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {

struct FaviconLoader::PImpl
{
	int mProxyType{WinHttp::SYSTEMSETTING};
	CString mProxyHost;
	CString mProxyUser;
	CString mProxyPassword;

	void Configure(WinHttp& http) const
	{
		http.SetProxyType(mProxyType);
		http.SetProxyCredential(mProxyHost, mProxyUser, mProxyPassword);
	}

	bool GetOrigin(const CString& url, CString& origin) const;
	bool FindIconURL(const CString& htmlURL, const std::vector<BYTE>& content, CString& iconURL) const;
	HICON CreateIcon(const std::vector<BYTE>& content) const;
};

bool FaviconLoader::PImpl::GetOrigin(const CString& url, CString& origin) const
{
	WCHAR host[2048]{};
	URL_COMPONENTS components{};
	components.dwStructSize = sizeof(components);
	components.lpszHostName = host;
	components.dwHostNameLength = static_cast<DWORD>(std::size(host));

	if (WinHttpCrackUrl(url, url.GetLength(), 0, &components) == FALSE ||
		(components.nScheme != INTERNET_SCHEME_HTTP && components.nScheme != INTERNET_SCHEME_HTTPS)) {
		return false;
	}

	CString scheme = components.nScheme == INTERNET_SCHEME_HTTPS ? _T("https") : _T("http");
	origin.Format(_T("%s://%s"), (LPCTSTR)scheme, host);
	if ((components.nScheme == INTERNET_SCHEME_HTTP && components.nPort != INTERNET_DEFAULT_HTTP_PORT) ||
		(components.nScheme == INTERNET_SCHEME_HTTPS && components.nPort != INTERNET_DEFAULT_HTTPS_PORT)) {
		CString port;
		port.Format(_T(":%u"), components.nPort);
		origin += port;
	}
	return true;
}

bool FaviconLoader::PImpl::FindIconURL(const CString& htmlURL, const std::vector<BYTE>& content, CString& iconURL) const
{
	if (content.empty()) {
		return false;
	}

	CString html(CStringA(reinterpret_cast<LPCSTR>(content.data()), static_cast<int>(content.size())));
	std::wstring source((LPCWSTR)html);
	static const std::wregex linkRegex(LR"(<link\b[^>]*>)", std::regex_constants::icase);
	static const std::wregex relRegex(LR"(\brel\s*=\s*["']([^"']*)["'])", std::regex_constants::icase);
	static const std::wregex hrefRegex(LR"(\bhref\s*=\s*["']([^"']+)["'])", std::regex_constants::icase);

	for (std::wsregex_iterator it(source.begin(), source.end(), linkRegex), end; it != end; ++it) {
		std::wstring tag = (*it)[0].str();
		std::wsmatch relMatch;
		std::wsmatch hrefMatch;
		if (!std::regex_search(tag, relMatch, relRegex) || !std::regex_search(tag, hrefMatch, hrefRegex)) {
			continue;
		}

		CString rel(relMatch[1].str().c_str());
		rel.MakeLower();
		if (rel.Find(_T("icon")) == -1) {
			continue;
		}

		CString href(hrefMatch[1].str().c_str());
		std::vector<TCHAR> combined(32768);
		DWORD length = static_cast<DWORD>(combined.size());
		if (UrlCombine(htmlURL, href, combined.data(), &length, 0) == S_OK) {
			iconURL = combined.data();
			return true;
		}
	}
	return false;
}

HICON FaviconLoader::PImpl::CreateIcon(const std::vector<BYTE>& content) const
{
	if (content.empty() || content.size() > static_cast<size_t>((std::numeric_limits<SIZE_T>::max)())) {
		return nullptr;
	}

	// ICO files contain one or more RT_ICON resources. Decode the largest one first.
	if (content.size() >= 6 && content[0] == 0 && content[1] == 0 &&
		content[2] == 1 && content[3] == 0) {
		WORD count = static_cast<WORD>(content[4] | (content[5] << 8));
		if (count > 0 && content.size() >= 6 + static_cast<size_t>(count) * 16) {
			const BYTE* bestEntry = nullptr;
		int bestArea = -1;
		for (WORD i = 0; i < count; ++i) {
			const BYTE* entry = content.data() + 6 + static_cast<size_t>(i) * 16;
			int width = entry[0] == 0 ? 256 : entry[0];
			int height = entry[1] == 0 ? 256 : entry[1];
			int area = width * height;
			if (area > bestArea) {
				bestArea = area;
				bestEntry = entry;
			}
		}
		if (bestEntry != nullptr) {
			DWORD resourceSize = bestEntry[8] | (bestEntry[9] << 8) |
				(bestEntry[10] << 16) | (bestEntry[11] << 24);
			DWORD resourceOffset = bestEntry[12] | (bestEntry[13] << 8) |
				(bestEntry[14] << 16) | (bestEntry[15] << 24);
			if (resourceSize > 0 && resourceOffset <= content.size() &&
				resourceSize <= content.size() - resourceOffset) {
				HICON icon = CreateIconFromResourceEx(
					const_cast<BYTE*>(content.data() + resourceOffset), resourceSize,
					TRUE, 0x00030000, 0, 0, LR_DEFAULTCOLOR);
				if (icon != nullptr) {
					return icon;
				}
			}
		}
	}
	}

	HGLOBAL global = GlobalAlloc(GMEM_MOVEABLE, content.size());
	if (global == nullptr) {
		return nullptr;
	}
	void* data = GlobalLock(global);
	if (data == nullptr) {
		GlobalFree(global);
		return nullptr;
	}
	memcpy(data, content.data(), content.size());
	GlobalUnlock(global);

	IStream* stream = nullptr;
	if (FAILED(CreateStreamOnHGlobal(global, TRUE, &stream))) {
		GlobalFree(global);
		return nullptr;
	}

	ATL::CImage image;
	HRESULT hr = image.Load(stream);
	stream->Release();
	if (FAILED(hr) || image.GetWidth() <= 0 || image.GetHeight() <= 0) {
		return nullptr;
	}

	ATL::CImage resized;
	HBITMAP bitmap = (HBITMAP)image;
	int width = image.GetWidth();
	int height = image.GetHeight();
	if (width > 64 || height > 64) {
		int size = std::min(width, height);
		int newWidth = width > height ? 64 : std::max(1, width * 64 / size);
		int newHeight = height > width ? 64 : std::max(1, height * 64 / size);
		if (resized.Create(newWidth, newHeight, image.GetBPP()) == FALSE) {
			return nullptr;
		}
		image.Draw(resized.GetDC(), 0, 0, newWidth, newHeight);
		resized.ReleaseDC();
		bitmap = (HBITMAP)resized;
		width = newWidth;
		height = newHeight;
	}

	ATL::CImage mask;
	if (mask.Create(width, height, 1) == FALSE) {
		return nullptr;
	}
	memset(mask.GetBits(), 0xff, abs(mask.GetPitch()) * height);

	ICONINFO info{};
	info.fIcon = TRUE;
	info.hbmMask = (HBITMAP)mask;
	info.hbmColor = bitmap;
	return CreateIconIndirect(&info);
}

FaviconLoader::FaviconLoader() : in(std::make_unique<PImpl>())
{
}

FaviconLoader::~FaviconLoader() = default;

void FaviconLoader::SetProxyType(int type)
{
	in->mProxyType = type;
}

void FaviconLoader::SetProxyCredential(const CString& host, const CString& user, const CString& password)
{
	in->mProxyHost = host;
	in->mProxyUser = user;
	in->mProxyPassword = password;
}

void FaviconLoader::SetProxy(const CString& host, WORD port, const CString& user, const CString& password)
{
	CString proxyHost = host;
	if (port != 0) {
		proxyHost.AppendFormat(_T(":%u"), port);
	}
	SetProxyCredential(proxyHost, user, password);
	in->mProxyType = WinHttp::DIRECTPROXY;
}

bool FaviconLoader::Load(const CString& url, std::vector<BYTE>& image)
{
	WinHttp http;
	in->Configure(http);

	CString origin;
	if (!in->GetOrigin(url, origin)) {
		return false;
	}

	std::vector<BYTE> html;
	bool isHTML = false;
	CString iconURL;
	if (http.LoadContent(url, html, isHTML) && isHTML) {
		in->FindIconURL(url, html, iconURL);
	}
	if (iconURL.IsEmpty()) {
		iconURL = origin + _T("/favicon.ico");
	}

	image.clear();
	if (!iconURL.IsEmpty() && http.LoadBinaryContent(iconURL, image)) {
		return true;
	}

	// A page can advertise an unavailable or unsupported icon. Try the conventional path.
	if (iconURL != origin + _T("/favicon.ico")) {
		image.clear();
		if (http.LoadBinaryContent(origin + _T("/favicon.ico"), image)) {
			return true;
		}
	}
	return false;
}

HICON FaviconLoader::Load(const CString& url)
{
	std::vector<BYTE> image;
	if (Load(url, image) == false) {
		return nullptr;
	}
	return in->CreateIcon(image);
}

} // end of namespace launcherapp
