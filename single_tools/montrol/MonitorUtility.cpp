#include "MonitorUtility.h"

#include <algorithm>
#include <charconv>
#include <cmath>
#include <cctype>
#include <map>

namespace {

const std::map<std::string, unsigned int> kInputAliases = {
	{"vga1", 1}, {"vga", 1}, {"dvi1", 3}, {"dvi", 3}, {"dvi2", 4},
	{"composite", 8}, {"svideo", 9}, {"s-video", 9}, {"dp1", 15}, {"dp", 15},
	{"displayport", 15}, {"dp2", 16}, {"hdmi1", 17}, {"hdmi", 17}, {"hdmi2", 18},
	{"usbc", 27}, {"usb-c", 27}, {"typec", 27}, {"type-c", 27}
};

/**
 * 入力ソース名を大文字小文字を区別しない比較用の小文字へ変換する
 *
 * @param[in] value 変換対象の文字列
 * @return 小文字化した文字列
 */
std::string ToLower(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
		return static_cast<char>(std::tolower(c));
	});
	return value;
}

/**
 * 文字列全体を指定した基数の符号なし整数として解析する
 *
 * @param[in] value 解析対象の文字列
 * @param[out] result 解析結果
 * @param[in] base 使用する基数
 * @return true:成功 false:不正な文字列
 */
bool ParseUnsigned(const std::string& value, unsigned int& result, int base)
{
	if (value.empty()) {
		return false;
	}
	unsigned int parsed = 0;
	auto converted = std::from_chars(value.data(), value.data() + value.size(), parsed, base);
	if (converted.ec != std::errc() || converted.ptr != value.data() + value.size()) {
		return false;
	}
	result = parsed;
	return true;
}

} // 名前空間

std::string GetInputSourceAlias(unsigned int value)
{
	// MCCS標準値に対応する表示名を返し、未知の値は呼び出し側で数値表示する。
	switch (value) {
	case 1: return "VGA1";
	case 3: return "DVI1";
	case 4: return "DVI2";
	case 8: return "Composite";
	case 9: return "S-Video";
	case 15: return "DisplayPort1";
	case 16: return "DisplayPort2";
	case 17: return "HDMI1";
	case 18: return "HDMI2";
	case 27: return "USB-C";
	default: return {};
	}
}

bool ParseInputSourceValue(const std::string& value, unsigned int& vcpValue)
{
	// まず標準エイリアスを検索し、見つからなければモニター固有値として数値を解析する。
	const std::string lower = ToLower(value);
	const auto alias = kInputAliases.find(lower);
	if (alias != kInputAliases.end()) {
		vcpValue = alias->second;
		return true;
	}
	if (!ParseUnsigned(value, vcpValue, 10)) {
		return false;
	}
	return vcpValue <= 255;
}

std::vector<InputSourceInfo> ParseInputSources(const std::string& capabilities)
{
	std::vector<InputSourceInfo> result;
	// MCCSケイパビリティ文字列のVCPコード60の括弧内を入力ソース一覧として解析する。
	const std::string marker = "60(";
	const auto begin = capabilities.find(marker);
	if (begin == std::string::npos) {
		return result;
	}
	const auto end = capabilities.find(')', begin + marker.size());
	if (end == std::string::npos) {
		return result;
	}

	std::size_t pos = begin + marker.size();
	while (pos < end) {
		while (pos < end && std::isspace(static_cast<unsigned char>(capabilities[pos]))) {
			++pos;
		}
		const std::size_t tokenBegin = pos;
		while (pos < end && !std::isspace(static_cast<unsigned char>(capabilities[pos]))) {
			++pos;
		}
		if (tokenBegin == pos) {
			continue;
		}
		unsigned int value = 0;
		// ケイパビリティ値は16進数で記述されるため、10進数のCLI入力とは別に解析する。
		if (ParseUnsigned(capabilities.substr(tokenBegin, pos - tokenBegin), value, 16) && value <= 255) {
			const std::string alias = GetInputSourceAlias(value);
			result.push_back({value, alias.empty() ? std::to_string(value) : alias});
		}
	}
	return result;
}

int NormalizeBrightness(unsigned int current, unsigned int minimum, unsigned int maximum)
{
	if (maximum <= minimum) {
		return 0;
	}
	// デバイスから返された値が範囲外でも、正規化結果が0～100を超えないようにする。
	const unsigned int clamped = std::min(std::max(current, minimum), maximum);
	return static_cast<int>(std::lround((static_cast<double>(clamped - minimum) * 100.0) / (maximum - minimum)));
}

bool DenormalizeBrightness(int value, unsigned int minimum, unsigned int maximum, unsigned int& deviceValue)
{
	if (value < 0 || value > 100 || maximum < minimum) {
		return false;
	}
	// ツールの百分率をデバイスの有効範囲へ戻し、小数点以下は四捨五入する。
	deviceValue = minimum + static_cast<unsigned int>(std::lround((maximum - minimum) * (value / 100.0)));
	return true;
}
