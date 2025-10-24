#include "pch.h"
#include "URLDirectoryIndexCommandParam.h"
#include "utility/AES.h"
#include <winhttp.h>
#include <array>
#include <vector>

namespace launcherapp {
namespace commands {
namespace url_directoryindex {

static std::vector<uint8_t>& Encode(const CString& str, std::vector<uint8_t>& buf);
static CString Decode(const std::vector<uint8_t>& src);

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

bool CommandParam::Save(CommandEntryIF* entry)
{
	entry->Set(_T("description"), mDescription);

	entry->Set(_T("url"), mURL);

	entry->Set(_T("serveruser"), mServerUser);
	std::vector<uint8_t> buf;
	auto& stm = Encode(mServerPassword, buf);
	entry->SetBytes(_T("serverpassword"), stm.data(), stm.size());

	entry->Set(_T("proxytype"), mProxyType);
	entry->Set(_T("proxyhost"), mProxyHost);
	entry->Set(_T("proxyuser"), mProxyUser);

	auto& stm2 = Encode(mProxyPassword, buf);
	entry->SetBytes(_T("proxypassword"), stm2.data(), stm2.size());

	return true;
}

bool CommandParam::Load(CommandEntryIF* entry)
{
	mName = entry->GetName();
	mDescription = entry->Get(_T("description"), _T(""));
	mURL = entry->Get(_T("url"), _T(""));

	mServerUser = entry->Get(_T("serveruser"), _T(""));

	size_t len = entry->GetBytesLength(_T("serverpassword"));
	if (len != CommandEntryIF::NO_ENTRY) {
		std::vector<uint8_t> buf(len);
		entry->GetBytes(_T("serverpassword"), buf.data(), len);
		mServerPassword = Decode(buf);
	}

	mProxyType = entry->Get(_T("proxytype"), 0);
	mProxyHost = entry->Get(_T("proxyhost"), _T(""));
	mProxyUser = entry->Get(_T("proxyuser"), _T(""));

	len = entry->GetBytesLength(_T("proxypassword"));
	if (len != CommandEntryIF::NO_ENTRY) {
		std::vector<uint8_t> buf(len);
		entry->GetBytes(_T("proxypassword"), buf.data(), len);
		mProxyPassword = Decode(buf);
	}

	return true;
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

std::vector<uint8_t>& Encode(const CString& str, std::vector<uint8_t>& buf)
{
	utility::aes::AES aes;
	aes.SetPassphrase("aiueo");  // てきとうa

	int len = str.GetLength() + 1;
	std::vector<uint8_t> plainData(len * sizeof(TCHAR));
	memcpy(plainData.data(), (LPCTSTR)str, plainData.size());

	aes.Encrypt(plainData, buf);
	return buf;
}

CString Decode(const std::vector<uint8_t>& src)
{
	utility::aes::AES aes;
	aes.SetPassphrase("aiueo");  // てきとう

	std::vector<uint8_t> plainData;
	if (aes.Decrypt(src, plainData) == false) {
		return _T("");
	}

	int len = (int)plainData.size();
	CString str;
	memcpy(str.GetBuffer(len), plainData.data(), len);
	str.ReleaseBuffer();

	return str;
}



} // end of namespace url_directoryindex
} // end of namespace commands
} // end of namespace launcherapp

