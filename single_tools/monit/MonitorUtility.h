#pragma once

#include <optional>
#include <string>
#include <vector>

struct InputSourceInfo
{
	unsigned int id;
	std::string displayName;
};

/**
 * VCP値に対応する入力ソース名を取得する
 *
 * @param[in] value VCP値
 * @return 入力ソース名。標準値にない場合は空文字列
 */
std::string GetInputSourceAlias(unsigned int value);

/**
 * 入力ソース名または数値をVCP値へ変換する
 *
 * @param[in] value 入力ソース名または数値文字列
 * @param[out] vcpValue 変換後のVCP値
 * @return true:成功 false:変換失敗
 */
bool ParseInputSourceValue(const std::string& value, unsigned int& vcpValue);

/**
 * MCCSケイパビリティ文字列から入力ソース一覧を取得する
 *
 * @param[in] capabilities MCCSケイパビリティ文字列
 * @return 入力ソース一覧
 */
std::vector<InputSourceInfo> ParseInputSources(const std::string& capabilities);

/**
 * デバイス値を0～100の輝度値へ変換する
 *
 * @param[in] current 現在値
 * @param[in] minimum デバイス最小値
 * @param[in] maximum デバイス最大値
 * @return 正規化された輝度値
 */
int NormalizeBrightness(unsigned int current, unsigned int minimum, unsigned int maximum);

/**
 * 正規化された輝度値をデバイス値へ変換する
 *
 * @param[in] value 正規化された輝度値
 * @param[in] minimum デバイス最小値
 * @param[in] maximum デバイス最大値
 * @param[out] deviceValue 変換後のデバイス値
 * @return true:成功 false:範囲外または不正な範囲
 */
bool DenormalizeBrightness(int value, unsigned int minimum, unsigned int maximum, unsigned int& deviceValue);
