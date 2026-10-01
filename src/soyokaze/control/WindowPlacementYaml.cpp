#include "pch.h"
#include "WindowPlacementYaml.h"
#include "WindowPosition.h"
#include "utility/Base64.h"
#define RYML_SINGLE_HDR_DEFINE_NOW
#include <rapidyaml/rapidyaml.hpp>
#include <stdexcept>
#include <string>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

constexpr size_t MAX_WINDOWPLACEMENT_YAML_SIZE = 1024 * 1024;
constexpr wchar_t MAIN_WINDOW_NAME[] = L"Window";

/** 読み書きの対象外とするキー */
bool IsDefaultKey(const ryml::csubstr& key)
{
	return key == "default";
}

bool IsDefaultKey(const std::wstring& key)
{
	return key == L"default";
}

/** ASCIIのみで構成されているか確認する(キーとして使用できない文字の判定に使用する) */
bool IsAsciiOnly(const ryml::csubstr& value)
{
	for (size_t i = 0; i < value.len; ++i) {
		if (static_cast<unsigned char>(value.str[i]) > 0x7f) {
			return false;
		}
	}
	return true;
}

std::wstring ToWideAscii(const ryml::csubstr& value)
{
	if (IsAsciiOnly(value) == false) {
		return std::wstring();
	}

	std::wstring result;
	result.reserve(value.len);
	for (size_t i = 0; i < value.len; ++i) {
		result.push_back(static_cast<wchar_t>(value.str[i]));
	}
	return result;
}

CString ToCStringAscii(const ryml::csubstr& value)
{
	CString result;
	for (size_t i = 0; i < value.len; ++i) {
		if (static_cast<unsigned char>(value.str[i]) > 0x7f) {
			return CString();
		}
		result.AppendChar(static_cast<TCHAR>(value.str[i]));
	}
	return result;
}

std::string ToAscii(const std::wstring& value)
{
	std::string result;
	result.reserve(value.size());
	for (wchar_t ch : value) {
		if (ch > 0x7f) {
			return std::string();
		}
		result.push_back(static_cast<char>(ch));
	}
	return result;
}

std::string EncodePlacement(const WINDOWPLACEMENT& placement)
{
	std::vector<uint8_t> bytes(sizeof(WINDOWPLACEMENT));
	memcpy(bytes.data(), &placement, sizeof(WINDOWPLACEMENT));

	CString encoded = utility::base64::EncodeBase64(bytes);
	std::string result;
	result.reserve(encoded.GetLength());
	for (int i = 0; i < encoded.GetLength(); ++i) {
		result.push_back(static_cast<char>(encoded[i]));
	}
	return result;
}

/** 単一のエントリをデコードし、有効な場合のみ位置情報を登録する */
bool AddPlacement(const ryml::csubstr& key, const ryml::csubstr& value, WindowPlacementYaml::PlacementMap& placements)
{
	if (IsDefaultKey(key)) {
		// defaultキーは現状読み書き対象外
		return false;
	}

	auto identifier = ToWideAscii(key);
	if (identifier.empty()) {
		return false;
	}

	CString encoded = ToCStringAscii(value);
	if (encoded.IsEmpty()) {
		return false;
	}

	std::vector<uint8_t> bytes;
	if (utility::base64::DecodeBase64(encoded, bytes) == false) {
		return false;
	}

	WINDOWPLACEMENT placement{};
	if (WindowPosition::IsValidWindowPlacementData(bytes, placement) == false) {
		return false;
	}

	placements[identifier] = placement;
	return true;
}

void OnYamlParseError(ryml::csubstr message, ryml::ErrorDataParse const& errorData, void* userData)
{
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(errorData);
	UNREFERENCED_PARAMETER(userData);
	throw std::runtime_error("Invalid window placement YAML");
}

}

WindowPlacementYaml::WindowPlacementYaml()
{
}

WindowPlacementYaml::~WindowPlacementYaml()
{
}

/**
  YAML文字列から位置情報を読み込む
  値がマップでないエントリは旧形式のフラット形式とみなし、メインウインドウの名前で扱う
  @return true:読み込んだ false:読み込めなかった(または引数が不正)
  @param[in] yaml 読み込むYAML文字列
  @param[out] result 読み込んだ位置情報
*/
bool WindowPlacementYaml::Parse(const std::string& yaml, WindowPlacementMap& result)
{
	if (yaml.empty() || yaml.size() > MAX_WINDOWPLACEMENT_YAML_SIZE) {
		return false;
	}

	try {
		ryml::Callbacks callbacks;
		callbacks.set_error_parse(OnYamlParseError);
		ryml::Tree tree(callbacks);
		ryml::EventHandlerTree handler(&tree, tree.root_id());
		ryml::Parser parser(&handler);
		ryml::parse_in_arena(&parser, ryml::to_csubstr(yaml), &tree, tree.root_id());

		auto root = tree.rootref();
		if (root.is_map() == false) {
			return false;
		}

		WindowPlacementMap parsed;
		for (auto child = root.first_child(); child.readable(); child = child.next_sibling()) {
			// 値がマップの場合はウインドウ名ごとの位置情報として扱う
			// (キーと値がマップのノードは、そのノードの子要素がマップのエントリとなる)
			if (child.is_map()) {
				auto windowName = ToWideAscii(child.key());
				if (windowName.empty()) {
					continue;
				}

				auto& placements = parsed[windowName];
				for (auto placementChild = child.first_child(); placementChild.readable(); placementChild = placementChild.next_sibling()) {
					if (placementChild.is_keyval() == false) {
						continue;
					}
					AddPlacement(placementChild.key(), placementChild.val(), placements);
				}
				continue;
			}

			// 値がスカラーの場合は旧形式のフラット形式として、メインウインドウ名下へ振り分ける
			if (child.is_keyval() == false) {
				continue;
			}
			if (IsDefaultKey(child.key())) {
				continue;
			}
			AddPlacement(child.key(), child.val(), parsed[MAIN_WINDOW_NAME]);
		}

		result.swap(parsed);
		return true;
	}
	catch (...) {
		result.clear();
		return false;
	}
}

/**
  位置情報をYAML文字列へ変換する
  キーに使用できない文字を含むウインドウがある場合は空文字列を返す
  @return YAML文字列。変換できなかった場合は空文字列
  @param[in] placements 変換する位置情報
*/
std::string WindowPlacementYaml::Emit(const WindowPlacementMap& placements)
{
	if (placements.empty()) {
		return std::string();
	}

	try {
		// rapidyamlは文字列のポインタを保持するため、木への設定に必要な領域を预先確保して
		// 文字列の寿命が出力完了まで失われないようにする
		size_t placementCount = 0;
		for (const auto& windowEntry : placements) {
			placementCount += windowEntry.second.size();
		}

		std::vector<std::string> windowNames;
		std::vector<std::string> identifiers;
		std::vector<std::string> values;
		windowNames.reserve(placements.size());
		identifiers.reserve(placementCount);
		values.reserve(placementCount);

		ryml::Tree tree;
		tree.rootref().set_map();

		// ウインドウごとに「構成識別子 -> Base64エンコードした位置情報」のマップを出力する
		for (const auto& windowEntry : placements) {
			auto windowName = ToAscii(windowEntry.first);
			if (windowName.empty()) {
				return std::string();
			}
			windowNames.push_back(std::move(windowName));

			ryml::NodeRef windowNode = tree.rootref()[ryml::to_csubstr(windowNames.back())];
			windowNode.set_map();

			for (const auto& entry : windowEntry.second) {
				auto identifier = ToAscii(entry.first);
				if (identifier.empty()) {
					return std::string();
				}
				identifiers.push_back(std::move(identifier));
				values.push_back(EncodePlacement(entry.second));

				windowNode[ryml::to_csubstr(identifiers.back())].set_val(ryml::to_csubstr(values.back()), ryml::VAL_DQUO);
			}
		}

		return ryml::emitrs_yaml<std::string>(tree);
	}
	catch (...) {
		return std::string();
	}
}

/**
  ウインドウ名をYAMLのキーとして使用できるか確認する
  @return true:使用できる false:使用できない
  @param[in] name ウインドウ名
*/
bool WindowPlacementYaml::IsValidWindowName(const std::wstring& name)
{
	return name.empty() ? false : ToAscii(name).empty() == false;
}

const wchar_t* WindowPlacementYaml::GetMainWindowName()
{
	return MAIN_WINDOW_NAME;
}