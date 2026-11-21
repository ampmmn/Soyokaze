#include "SearchViewerConfig.h"

#define RYML_SINGLE_HDR_DEFINE_NOW
#include <rapidyaml/rapidyaml.hpp>

#include <Windows.h>
#include <Winhttp.h>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <map>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")

namespace {

constexpr size_t MAX_CONFIG_SIZE = 1024 * 1024;

/** UTF-8文字列をワイド文字列へ変換する */
bool ToWide(const std::string& value, std::wstring& result)
{
	if (value.empty()) {
		result.clear();
		return true;
	}
	const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0);
	if (length <= 0) {
		return false;
	}
	std::wstring converted(static_cast<size_t>(length), L'\0');
	if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), converted.data(), length) != length) {
		return false;
	}
	result.swap(converted);
	return true;
}

/** ワイド文字列をUTF-8へ変換する */
bool ToUtf8(const std::wstring& value, std::string& result)
{
	if (value.empty()) {
		result.clear();
		return true;
	}
	const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
	if (length <= 0) {
		return false;
	}
	std::string converted(static_cast<size_t>(length), '\0');
	if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()), converted.data(), length, nullptr, nullptr) != length) {
		return false;
	}
	result.swap(converted);
	return true;
}

/** YAMLノードが文字列値であることを確認して取得する */
bool ReadNodeString(const ryml::ConstNodeRef& node, std::wstring& result)
{
	if (node.is_keyval() == false || node.has_val() == false) {
		return false;
	}
	const auto value = node.val();
	return ToWide(std::string(value.str, value.len), result);
}

/** プロファイル名に使えるASCII文字か確認する */
bool IsNameCharacter(wchar_t character)
{
	return (character >= L'a' && character <= L'z') ||
		(character >= L'A' && character <= L'Z') ||
		(character >= L'0' && character <= L'9') || character == L'_' || character == L'-';
}

/** 検索語をUTF-8のURLエンコード形式へ変換する */
bool EncodeQuery(const std::wstring& query, std::wstring& encoded)
{
	std::string utf8;
	if (ToUtf8(query, utf8) == false) {
		return false;
	}
	static constexpr wchar_t HEX[] = L"0123456789ABCDEF";
	std::wstring result;
	result.reserve(utf8.size() * 3);
	for (const unsigned char byte : utf8) {
		if ((byte >= 'a' && byte <= 'z') || (byte >= 'A' && byte <= 'Z') ||
			(byte >= '0' && byte <= '9') || byte == '-' || byte == '.' || byte == '_' || byte == '~') {
			result.push_back(static_cast<wchar_t>(byte));
		}
		else {
			result.push_back(L'%');
			result.push_back(HEX[(byte >> 4) & 0x0f]);
			result.push_back(HEX[byte & 0x0f]);
		}
	}
	encoded.swap(result);
	return true;
}

/** URLがHTTPまたはHTTPSスキームを持ち、制御文字を含まないか確認する */
bool HasSafeUrlScheme(const std::wstring& url)
{
	if (url.size() < 8) {
		return false;
	}
	const bool http = CompareStringOrdinal(url.data(), 7, L"http://", 7, TRUE) == CSTR_EQUAL;
	const bool https = url.size() >= 8 && CompareStringOrdinal(url.data(), 8, L"https://", 8, TRUE) == CSTR_EQUAL;
	if (http == false && https == false) {
		return false;
	}

	for (const wchar_t character : url) {
		if (character <= L' ' || character == L'\r' || character == L'\n' || character == L'\t') {
			return false;
		}
	}
	return true;
}

} // 名前空間

struct SearchViewerConfig::PImpl
{
	struct Profile
	{
		std::wstring site;
		std::wstring title;
	};
	std::map<std::wstring, Profile> profiles;
	std::map<std::wstring, std::wstring> sites;
};

SearchViewerConfig::SearchViewerConfig() : in(std::make_unique<PImpl>())
{
}

SearchViewerConfig::~SearchViewerConfig() = default;
SearchViewerConfig::SearchViewerConfig(SearchViewerConfig&&) noexcept = default;
SearchViewerConfig& SearchViewerConfig::operator=(SearchViewerConfig&&) noexcept = default;

bool SearchViewerConfig::Parse(const std::string& yaml, SearchViewerConfig& result, std::wstring& error)
{
	error.clear();
	if (yaml.empty() || yaml.size() > MAX_CONFIG_SIZE) {
		error = L"設定ファイルが空か、サイズ上限を超えています。";
		return false;
	}

	try {
		auto tree = ryml::parse_in_arena(ryml::to_csubstr(yaml));
		const auto root = tree.rootref();
		if (root.is_map() == false) {
			error = L"設定ファイルのルートはマップである必要があります。";
			return false;
		}

		SearchViewerConfig parsed;
		const auto profiles = root.find_child("profiles");
		const auto sites = root.find_child("sites");
		if (profiles.readable() == false || profiles.is_map() == false || sites.readable() == false || sites.is_map() == false) {
			error = L"profiles と sites の定義が必要です。";
			return false;
		}

		for (auto child = profiles.first_child(); child.readable(); child = child.next_sibling()) {
			std::wstring profileName;
			if (ToWide(std::string(child.key().str, child.key().len), profileName) == false || IsValidProfileName(profileName) == false || child.is_map() == false) {
				error = L"不正なプロファイル定義があります。";
				return false;
			}

			std::wstring siteName;
			std::wstring title;
			if (ReadNodeString(child.find_child("site"), siteName) == false || ReadNodeString(child.find_child("title"), title) == false || title.empty()) {
				error = L"プロファイルには site と title の文字列が必要です。";
				return false;
			}
			parsed.in->profiles.emplace(std::move(profileName), PImpl::Profile{ std::move(siteName), std::move(title) });
		}

		for (auto child = sites.first_child(); child.readable(); child = child.next_sibling()) {
			std::wstring siteName;
			if (ToWide(std::string(child.key().str, child.key().len), siteName) == false || siteName.empty() || child.is_map() == false) {
				error = L"不正なサイト定義があります。";
				return false;
			}
			std::wstring searchUrl;
			if (ReadNodeString(child.find_child("searchUrl"), searchUrl) == false || searchUrl.find(L"{query}") == std::wstring::npos || HasSafeUrlScheme(searchUrl) == false) {
				error = L"サイトにはHTTPまたはHTTPSの searchUrl と {query} が必要です。";
				return false;
			}
			parsed.in->sites.emplace(std::move(siteName), std::move(searchUrl));
		}

		if (parsed.in->profiles.empty() || parsed.in->sites.empty()) {
			error = L"プロファイルとサイトをそれぞれ1件以上定義してください。";
			return false;
		}
		for (const auto& profile : parsed.in->profiles) {
			if (parsed.in->sites.find(profile.second.site) == parsed.in->sites.end()) {
				error = L"Unknown site: " + profile.second.site;
				return false;
			}
		}

		result = std::move(parsed);
		return true;
	}
	catch (...) {
		error = L"YAMLを解析できません。";
		return false;
	}
}

bool SearchViewerConfig::LoadFile(const std::wstring& path, SearchViewerConfig& result, std::wstring& error)
{
	std::ifstream file(path, std::ios::binary);
	if (file.is_open() == false) {
		error = L"設定ファイルを開けません: " + path;
		return false;
	}
	std::string yaml((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	return Parse(yaml, result, error);
}

bool SearchViewerConfig::GetTitle(const std::wstring& profile, std::wstring& title) const
{
	const auto it = in->profiles.find(profile);
	if (it == in->profiles.end()) {
		return false;
	}
	title = it->second.title;
	return true;
}

bool SearchViewerConfig::BuildSearchUrl(const std::wstring& profile, const std::wstring& query, std::wstring& url, std::wstring& error) const
{
	const auto profileEntry = in->profiles.find(profile);
	if (profileEntry == in->profiles.end()) {
		error = L"Unknown profile: " + profile;
		return false;
	}
	const auto siteEntry = in->sites.find(profileEntry->second.site);
	if (siteEntry == in->sites.end()) {
		error = L"Unknown site: " + profileEntry->second.site;
		return false;
	}
	std::wstring encodedQuery;
	if (EncodeQuery(query, encodedQuery) == false) {
		error = L"検索語をUTF-8に変換できません。";
		return false;
	}
	std::wstring result = siteEntry->second;
	size_t position = 0;
	while ((position = result.find(L"{query}", position)) != std::wstring::npos) {
		result.replace(position, 7, encodedQuery);
		position += encodedQuery.size();
	}
	if (IsAllowedUrl(result) == false) {
		error = L"生成されたURLはHTTPまたはHTTPSではありません。";
		return false;
	}
	url.swap(result);
	error.clear();
	return true;
}

bool SearchViewerConfig::IsAllowedUrl(const std::wstring& url)
{
	if (HasSafeUrlScheme(url) == false) {
		return false;
	}
	URL_COMPONENTS components{};
	components.dwStructSize = sizeof(components);
	components.dwSchemeLength = static_cast<DWORD>(-1);
	components.dwHostNameLength = static_cast<DWORD>(-1);
	components.dwUrlPathLength = static_cast<DWORD>(-1);
	components.dwExtraInfoLength = static_cast<DWORD>(-1);
	if (WinHttpCrackUrl(url.c_str(), static_cast<DWORD>(url.size()), 0, &components) == FALSE) {
		return false;
	}
	return (components.nScheme == INTERNET_SCHEME_HTTP || components.nScheme == INTERNET_SCHEME_HTTPS) && components.dwHostNameLength > 0;
}

bool SearchViewerConfig::IsValidProfileName(const std::wstring& profile)
{
	return profile.empty() == false && profile.size() <= 16 && std::all_of(profile.begin(), profile.end(), IsNameCharacter);
}
