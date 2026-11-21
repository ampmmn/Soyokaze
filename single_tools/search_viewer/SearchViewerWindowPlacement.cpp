#include "SearchViewerWindowPlacement.h"

#include <rapidyaml/rapidyaml.hpp>

#include <Wincrypt.h>

#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "crypt32.lib")

namespace {

bool ToAscii(const std::wstring& value, std::string& result)
{
	std::string converted;
	converted.reserve(value.size());
	for (const wchar_t character : value) {
		if (character > 0x7f) {
			return false;
		}
		converted.push_back(static_cast<char>(character));
	}
	result.swap(converted);
	return true;
}

/** Base64で復元した表示状態が通常のウインドウ状態か確認する */
bool IsSafeShowCommand(UINT command)
{
	return command == SW_SHOWNORMAL || command == SW_SHOWMINIMIZED ||
		command == SW_SHOWMAXIMIZED || command == SW_SHOWNOACTIVATE || command == SW_SHOW ||
		command == SW_MINIMIZE || command == SW_SHOWMINNOACTIVE || command == SW_SHOWNA ||
		command == SW_RESTORE || command == SW_SHOWDEFAULT;
}

/** 1プロファイル分のウインドウ位置を含むYAMLを生成する */
std::string EmitSinglePlacement(const std::string& profileName, const std::string& encoded)
{
	ryml::Tree tree;
	tree.rootref().set_map();
	auto profileNode = tree.rootref()[ryml::to_csubstr(profileName)];
	profileNode.set_map();
	profileNode["default"].set_val(ryml::to_csubstr(encoded), ryml::VAL_DQUO);
	return ryml::emitrs_yaml<std::string>(tree);
}

} // 名前空間

std::string SearchViewerWindowPlacement::Encode(const WINDOWPLACEMENT& placement)
{
	std::vector<BYTE> bytes(sizeof(WINDOWPLACEMENT));
	memcpy(bytes.data(), &placement, sizeof(WINDOWPLACEMENT));
	DWORD length = 0;
	if (CryptBinaryToStringA(bytes.data(), static_cast<DWORD>(bytes.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &length) == FALSE) {
		return {};
	}
	std::string result(length, '\0');
	if (CryptBinaryToStringA(bytes.data(), static_cast<DWORD>(bytes.size()), CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, result.data(), &length) == FALSE) {
		return {};
	}
	if (length > 0 && result[length - 1] == '\0') {
		result.resize(length - 1);
	}
	else {
		result.resize(length);
	}
	return result;
}

bool SearchViewerWindowPlacement::Decode(const std::string& encoded, WINDOWPLACEMENT& placement)
{
	if (encoded.empty()) {
		return false;
	}
	DWORD length = 0;
	if (CryptStringToBinaryA(encoded.c_str(), static_cast<DWORD>(encoded.size()), CRYPT_STRING_BASE64, nullptr, &length, nullptr, nullptr) == FALSE || length != sizeof(WINDOWPLACEMENT)) {
		return false;
	}
	std::vector<BYTE> bytes(length);
	if (CryptStringToBinaryA(encoded.c_str(), static_cast<DWORD>(encoded.size()), CRYPT_STRING_BASE64, bytes.data(), &length, nullptr, nullptr) == FALSE) {
		return false;
	}
	WINDOWPLACEMENT decoded{};
	memcpy(&decoded, bytes.data(), sizeof(decoded));
	if (decoded.length != sizeof(WINDOWPLACEMENT) || IsSafeShowCommand(decoded.showCmd) == false) {
		return false;
	}
	placement = decoded;
	return true;
}

bool SearchViewerWindowPlacement::Parse(const std::string& yaml, const std::wstring& profile, WINDOWPLACEMENT& placement)
{
	if (yaml.empty() || yaml.size() > 1024 * 1024) {
		return false;
	}
	std::string profileName;
	if (ToAscii(profile, profileName) == false) {
		return false;
	}
	try {
		auto tree = ryml::parse_in_arena(ryml::to_csubstr(yaml));
		const auto root = tree.rootref();
		if (root.is_map() == false) {
			return false;
		}
		const auto profileNode = root.find_child(ryml::to_csubstr(profileName));
		if (profileNode.readable() == false || profileNode.is_map() == false) {
			return false;
		}
		const auto defaultNode = profileNode.find_child("default");
		if (defaultNode.readable() == false || defaultNode.is_keyval() == false || defaultNode.has_val() == false) {
			return false;
		}
		const auto encoded = defaultNode.val();
		return Decode(std::string(encoded.str, encoded.len), placement);
	}
	catch (...) {
		return false;
	}
}

std::string SearchViewerWindowPlacement::Update(const std::string& yaml, const std::wstring& profile, const WINDOWPLACEMENT& placement)
{
	std::string profileName;
	if (ToAscii(profile, profileName) == false || profileName.empty()) {
		return {};
	}
	const std::string encoded = Encode(placement);
	if (encoded.empty()) {
		return {};
	}
	if (yaml.empty()) {
		try {
			return EmitSinglePlacement(profileName, encoded);
		}
		catch (...) {
			return {};
		}
	}
	try {
		auto tree = ryml::parse_in_arena(ryml::to_csubstr(yaml));
		if (tree.rootref().is_map() == false) {
			return EmitSinglePlacement(profileName, encoded);
		}
		auto profileNode = tree.rootref().find_child(ryml::to_csubstr(profileName));
		if (profileNode.readable() == false) {
			profileNode = tree.rootref()[ryml::to_csubstr(profileName)];
			profileNode.set_map();
		}
		else if (profileNode.is_map() == false) {
			profileNode.set_map();
		}
		profileNode["default"].set_val(ryml::to_csubstr(encoded), ryml::VAL_DQUO);
		return ryml::emitrs_yaml<std::string>(tree);
	}
	catch (...) {
		try {
			return EmitSinglePlacement(profileName, encoded);
		}
		catch (...) {
			return {};
		}
	}
}
