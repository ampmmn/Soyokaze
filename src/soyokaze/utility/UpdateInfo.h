#pragma once

#include <cstdint>
#include <vector>

namespace launcherapp {
namespace utility {

struct VersionNumber
{
	uint32_t mMajor{0};
	uint32_t mMinor{0};
	uint32_t mBuild{0};
};

struct UpdateInfo
{
	VersionNumber mVersion;
	CString mDate;
	CString mUrl;
};

/**
  major.minor.build形式のバージョン文字列を数値に変換する
  @param[in] versionStr バージョン文字列
  @param[out] version 変換後のバージョン
  @return true:成功 false:失敗
*/
bool ParseVersionNumber(const CString& versionStr, VersionNumber& version);

/**
  2つのバージョンをmajor、minor、buildの順に比較する
  @param[in] lhs 比較元のバージョン
  @param[in] rhs 比較先のバージョン
  @return lhsがrhsより新しい場合はtrue
*/
bool IsVersionNewer(const VersionNumber& lhs, const VersionNumber& rhs);

/**
  更新情報JSONを解析する
  @param[in] content JSONデータ
  @param[out] updateInfo 解析した更新情報
  @return true:成功 false:失敗
*/
bool ParseUpdateInfo(const std::vector<BYTE>& content, UpdateInfo& updateInfo);

} // namespace utility
} // namespace launcherapp
