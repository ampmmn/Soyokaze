#include "pch.h"
#include "UpdateInfo.h"
#include <nlohmann/json.hpp>
#include <winhttp.h>
#include <chrono>
#include <limits>
#include <string>

#pragma comment(lib, "winhttp.lib")

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace utility {

using json = nlohmann::json;

namespace {

/**
  数字だけで構成されるバージョン要素を解析する
  @param[in] versionStr バージョン文字列
  @param[in] start 開始位置
  @param[in] length 要素の長さ
  @param[out] value 解析した数値
  @return true:成功 false:失敗
*/
bool ParseVersionPart(const CString& versionStr, int start, int length, uint32_t& value)
{
	if (length <= 0) {
		return false;
	}

	uint32_t parsedValue = 0;
	for (int i = 0; i < length; ++i) {
		TCHAR ch = versionStr[start + i];
		if (ch < _T('0') || ch > _T('9')) {
			return false;
		}

		uint32_t digit = static_cast<uint32_t>(ch - _T('0'));
		if (parsedValue > (std::numeric_limits<uint32_t>::max() - digit) / 10) {
			return false;
		}
		parsedValue = parsedValue * 10 + digit;
	}

	value = parsedValue;
	return true;
}

/**
  UTF-8文字列をUnicode文字列に変換する
  @param[in] source 変換元の文字列
  @param[out] result 変換後の文字列
  @return true:成功 false:失敗
*/
bool ConvertUtf8String(const std::string& source, CString& result)
{
	if (source.empty()) {
		result.Empty();
		return true;
	}
	if (source.size() > static_cast<size_t>(std::numeric_limits<int>::max())) {
		return false;
	}

	int sourceLength = static_cast<int>(source.size());
	int resultLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, source.data(), sourceLength, nullptr, 0);
	if (resultLength <= 0) {
		return false;
	}

	CStringW converted;
	WCHAR* buffer = converted.GetBuffer(resultLength);
	int convertedLength = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, source.data(), sourceLength, buffer, resultLength);
	converted.ReleaseBuffer(convertedLength);
	if (convertedLength != resultLength) {
		return false;
	}

	result = converted;
	return true;
}

/**
  日付文字列がYYYY-MM-DD形式として有効か確認する
  @param[in] date 日付文字列
  @return 有効な日付の場合はtrue
*/
bool IsValidDate(const std::string& date)
{
	if (date.size() != 10 || date[4] != '-' || date[7] != '-') {
		return false;
	}
	for (size_t i = 0; i < date.size(); ++i) {
		if (i == 4 || i == 7) {
			continue;
		}
		if (date[i] < '0' || date[i] > '9') {
			return false;
		}
	}

	int year = std::stoi(date.substr(0, 4));
	unsigned int month = static_cast<unsigned int>(std::stoi(date.substr(5, 2)));
	unsigned int day = static_cast<unsigned int>(std::stoi(date.substr(8, 2)));
	return std::chrono::year_month_day{
		std::chrono::year{year},
		std::chrono::month{month},
		std::chrono::day{day}
	}.ok();
}

/**
  HTTPS URLとして解析できることを確認する
  @param[in] url URL文字列
  @return 有効なHTTPS URLの場合はtrue
*/
bool IsValidHttpsUrl(const CString& url)
{
	WCHAR hostName[512]{};
	URL_COMPONENTS components{};
	components.dwStructSize = sizeof(components);
	components.lpszHostName = hostName;
	components.dwHostNameLength = static_cast<DWORD>(_countof(hostName));

	return WinHttpCrackUrl(url, url.GetLength(), 0, &components) != FALSE &&
		components.nScheme == INTERNET_SCHEME_HTTPS && components.dwHostNameLength > 0;
}

} // namespace

bool ParseVersionNumber(const CString& versionStr, VersionNumber& version)
{
	int firstDot = versionStr.Find(_T('.'));
	if (firstDot <= 0) {
		return false;
	}
	int secondDot = versionStr.Find(_T('.'), firstDot + 1);
	if (secondDot <= firstDot + 1 || secondDot == versionStr.GetLength() - 1 ||
		versionStr.Find(_T('.'), secondDot + 1) != -1) {
		return false;
	}

	VersionNumber parsedVersion;
	if (ParseVersionPart(versionStr, 0, firstDot, parsedVersion.mMajor) == false ||
		ParseVersionPart(versionStr, firstDot + 1, secondDot - firstDot - 1, parsedVersion.mMinor) == false ||
		ParseVersionPart(versionStr, secondDot + 1, versionStr.GetLength() - secondDot - 1, parsedVersion.mBuild) == false) {
		return false;
	}

	version = parsedVersion;
	return true;
}

bool IsVersionNewer(const VersionNumber& lhs, const VersionNumber& rhs)
{
	if (lhs.mMajor != rhs.mMajor) {
		return lhs.mMajor > rhs.mMajor;
	}
	if (lhs.mMinor != rhs.mMinor) {
		return lhs.mMinor > rhs.mMinor;
	}
	return lhs.mBuild > rhs.mBuild;
}

bool ParseUpdateInfo(const std::vector<BYTE>& content, UpdateInfo& updateInfo)
{
	try {
		json root = json::parse(content.begin(), content.end());
		if (root.is_object() == false) {
			return false;
		}

		auto latestIt = root.find("latest");
		if (latestIt == root.end() || latestIt->is_object() == false) {
			return false;
		}

		auto versionIt = latestIt->find("version");
		auto urlIt = latestIt->find("url");
		if (versionIt == latestIt->end() || versionIt->is_string() == false ||
			urlIt == latestIt->end() || urlIt->is_string() == false) {
			return false;
		}

		UpdateInfo parsedInfo;
		CString versionStr;
		if (ConvertUtf8String(versionIt->get<std::string>(), versionStr) == false ||
			ParseVersionNumber(versionStr, parsedInfo.mVersion) == false) {
			return false;
		}
		if (ConvertUtf8String(urlIt->get<std::string>(), parsedInfo.mUrl) == false ||
			IsValidHttpsUrl(parsedInfo.mUrl) == false) {
			return false;
		}

		auto dateIt = latestIt->find("date");
		if (dateIt != latestIt->end()) {
			if (dateIt->is_string() == false) {
				return false;
			}
			std::string date = dateIt->get<std::string>();
			if (IsValidDate(date) == false || ConvertUtf8String(date, parsedInfo.mDate) == false) {
				return false;
			}
		}

		updateInfo = parsedInfo;
		return true;
	}
	catch (const json::exception&) {
		return false;
	}
	catch (const std::exception&) {
		return false;
	}
}

} // namespace utility
} // namespace launcherapp
