#include "pch.h"
#include "WindowPosition.h"
#include "utility/AppProfile.h"
#include "utility/Path.h"
#include "utility/Base64.h"
#include "utility/SHA1.h"
#include "app/AppName.h"
#define RYML_SINGLE_HDR_DEFINE_NOW
#include <rapidyaml/rapidyaml.hpp>
#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

constexpr size_t MAX_WINDOWPLACEMENT_YAML_SIZE = 1024 * 1024;

using PlacementMap = std::map<std::wstring, WINDOWPLACEMENT>;

struct MonitorEnumerationData
{
	std::vector<RECT>* mRects;
};

struct MonitorCheckData
{
	RECT mWindowRect;
	bool mIsVisible{false};
};

BOOL CALLBACK EnumerateMonitorRectangles(HMONITOR monitor, HDC dc, LPRECT rect, LPARAM param)
{
	UNREFERENCED_PARAMETER(monitor);
	UNREFERENCED_PARAMETER(dc);

	auto data = reinterpret_cast<MonitorEnumerationData*>(param);
	data->mRects->push_back(*rect);
	return TRUE;
}

BOOL CALLBACK CheckWindowIntersectsMonitor(HMONITOR monitor, HDC dc, LPRECT rect, LPARAM param)
{
	UNREFERENCED_PARAMETER(monitor);
	UNREFERENCED_PARAMETER(dc);

	auto data = reinterpret_cast<MonitorCheckData*>(param);
	RECT intersection;
	if (IntersectRect(&intersection, rect, &data->mWindowRect)) {
		data->mIsVisible = true;
	}
	return TRUE;
}

bool IsWindowOnAnyMonitor(HWND hwnd)
{
	RECT windowRect;
	if (GetWindowRect(hwnd, &windowRect) == FALSE) {
		return false;
	}

	MonitorCheckData data{windowRect};
	EnumDisplayMonitors(nullptr, nullptr, CheckWindowIntersectsMonitor, reinterpret_cast<LPARAM>(&data));
	return data.mIsVisible;
}

CString GetWindowPlacementFilePath()
{
	std::vector<TCHAR> path(MAX_PATH_NTFS);
	CAppProfile::GetDirPath(path.data(), path.size(), true);
	Path filePath(path.data());
	filePath.Append(_T("windowplacement"));
	filePath.AddExtension(_T(".yaml"));
	return CString((LPCTSTR)filePath);
}

std::wstring ToWideAscii(const ryml::csubstr& value)
{
	std::wstring result;
	result.reserve(value.len);
	for (size_t i = 0; i < value.len; ++i) {
		if (static_cast<unsigned char>(value.str[i]) > 0x7f) {
			return std::wstring();
		}
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

void OnYamlParseError(ryml::csubstr message, ryml::ErrorDataParse const& errorData, void* userData)
{
	UNREFERENCED_PARAMETER(message);
	UNREFERENCED_PARAMETER(errorData);
	UNREFERENCED_PARAMETER(userData);
	throw std::runtime_error("Invalid window placement YAML");
}

bool ReadPlacementYaml(LPCTSTR filePath, PlacementMap& placements)
{
	try {
		CFile file;
		if (file.Open(filePath, CFile::modeRead | CFile::shareDenyWrite) == FALSE) {
			return false;
		}

		ULONGLONG fileSize = file.GetLength();
		if (fileSize == 0 || fileSize > MAX_WINDOWPLACEMENT_YAML_SIZE) {
			return false;
		}

		std::string yaml(static_cast<size_t>(fileSize), '\0');
		if (file.Read(yaml.data(), static_cast<UINT>(yaml.size())) != yaml.size()) {
			return false;
		}

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

		PlacementMap parsedPlacements;
		for (auto child = root.first_child(); child.readable(); child = child.next_sibling()) {
			if (child.is_keyval() == false) {
				continue;
			}

			auto key = ToWideAscii(child.key());
			if (key.empty() || key == L"default") {
				continue;
			}

			auto encodedPlacement = ToCStringAscii(child.val());
			if (encodedPlacement.IsEmpty()) {
				continue;
			}

			std::vector<uint8_t> bytes;
			if (utility::base64::DecodeBase64(encodedPlacement, bytes) == false) {
				continue;
			}

			WINDOWPLACEMENT placement{};
			if (WindowPosition::IsValidWindowPlacementData(bytes, placement)) {
				parsedPlacements[key] = placement;
			}
		}

		placements.swap(parsedPlacements);
		return true;
	}
	catch (...) {
		placements.clear();
		return false;
	}
}

bool WritePlacementYaml(LPCTSTR filePath, const PlacementMap& placements)
{
	try {
		ryml::Tree tree;
		tree.rootref().set_map();

		std::vector<std::string> keys;
		std::vector<std::string> values;
		keys.reserve(placements.size());
		values.reserve(placements.size());
		for (const auto& entry : placements) {
			if (entry.first == L"default") {
				continue;
			}

			std::string key;
			key.reserve(entry.first.size());
			for (wchar_t ch : entry.first) {
				if (ch > 0x7f) {
					return false;
				}
				key.push_back(static_cast<char>(ch));
			}

			std::vector<uint8_t> bytes(sizeof(WINDOWPLACEMENT));
			memcpy(bytes.data(), &entry.second, sizeof(WINDOWPLACEMENT));
			CString encoded = utility::base64::EncodeBase64(bytes);
			std::string value;
			value.reserve(encoded.GetLength());
			for (int i = 0; i < encoded.GetLength(); ++i) {
				value.push_back(static_cast<char>(encoded[i]));
			}

			keys.push_back(std::move(key));
			values.push_back(std::move(value));
		}

		for (size_t i = 0; i < keys.size(); ++i) {
			tree.rootref()[ryml::to_csubstr(keys[i])].set_val(ryml::to_csubstr(values[i]), ryml::VAL_DQUO);
		}

		std::string yaml = ryml::emitrs_yaml<std::string>(tree);
		CFile file(filePath, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
		file.Write(yaml.data(), static_cast<UINT>(yaml.size()));
		return true;
	}
	catch (...) {
		return false;
	}
}

bool ReadLegacyPlacement(LPCTSTR filePath, WINDOWPLACEMENT& placement)
{
	try {
		CFile file;
		if (file.Open(filePath, CFile::modeRead | CFile::shareDenyWrite) == FALSE ||
		    file.GetLength() != sizeof(WINDOWPLACEMENT)) {
			return false;
		}

		std::vector<uint8_t> bytes(sizeof(WINDOWPLACEMENT));
		if (file.Read(bytes.data(), static_cast<UINT>(bytes.size())) != bytes.size()) {
			return false;
		}
		return WindowPosition::IsValidWindowPlacementData(bytes, placement);
	}
	catch (...) {
		return false;
	}
}

bool ApplyPlacement(HWND hwnd, const WINDOWPLACEMENT& placement)
{
	WINDOWPLACEMENT previousPlacement{};
	previousPlacement.length = sizeof(WINDOWPLACEMENT);
	bool hasPreviousPlacement = GetWindowPlacement(hwnd, &previousPlacement) != FALSE;
	bool wasVisible = IsWindowVisible(hwnd) != FALSE;
	if (wasVisible == false) {
		// 保存位置のshowCmdで、非表示中のウインドウを表示しない
		previousPlacement.showCmd = SW_HIDE;
	}

	WINDOWPLACEMENT placementToApply = placement;
	if (wasVisible == false) {
		placementToApply.showCmd = SW_HIDE;
	}

	if (SetWindowPlacement(hwnd, &placementToApply) == FALSE) {
		return false;
	}
	if (IsWindowOnAnyMonitor(hwnd)) {
		return true;
	}
	if (hasPreviousPlacement) {
		SetWindowPlacement(hwnd, &previousPlacement);
	}
	return false;
}

}

struct WindowPosition::PImpl
{
	CString mName;
	WINDOWPLACEMENT mPosition{};
	bool mIsLoaded{false};
	PlacementMap mPlacements;
	std::wstring mCurrentMonitorConfiguration;
};

WindowPosition::WindowPosition() : in(std::make_unique<PImpl>())
{
	in->mName = _T("Window");
	in->mPosition.length = sizeof(WINDOWPLACEMENT);
	in->mPosition.showCmd = SW_SHOWNORMAL;
}

WindowPosition::WindowPosition(LPCTSTR name) : in(std::make_unique<PImpl>())
{
	ASSERT(name);
	in->mName = name;
	in->mPosition.length = sizeof(WINDOWPLACEMENT);
	in->mPosition.showCmd = SW_SHOWNORMAL;
}

WindowPosition::~WindowPosition()
{
	Save();
}

/**
  モニター構成からウインドウ位置を復元する
  復元した位置がモニター領域に収まっていない場合はfalseを返す
  @return true:復元した false:復元しなかった
  @param[in] hwnd 対象ウインドウハンドル
*/
bool WindowPosition::Restore(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		return false;
	}

	in->mPlacements.clear();
	std::vector<RECT> rects;
	MonitorEnumerationData data{&rects};
	EnumDisplayMonitors(nullptr, nullptr, EnumerateMonitorRectangles, reinterpret_cast<LPARAM>(&data));
	in->mCurrentMonitorConfiguration = (LPCTSTR)CreateMonitorConfigurationIdentifier(rects);

	CString yamlPath = GetWindowPlacementFilePath();
	if (Path::FileExists((LPCWSTR)yamlPath)) {
		if (ReadPlacementYaml(yamlPath, in->mPlacements)) {
			auto current = in->mPlacements.find(in->mCurrentMonitorConfiguration);
			if (current != in->mPlacements.end() && ApplyPlacement(hwnd, current->second)) {
				in->mPosition = current->second;
				in->mIsLoaded = true;
				return true;
			}

			return false;
		}
	}

	Path legacyPath;
	GetFilePath(in->mName, legacyPath);
	WINDOWPLACEMENT legacyPlacement{};
	if (ReadLegacyPlacement(legacyPath, legacyPlacement) == false) {
		GetFilePath(APPNAME, legacyPath);
		if (ReadLegacyPlacement(legacyPath, legacyPlacement) == false) {
			return false;
		}
	}

	if (ApplyPlacement(hwnd, legacyPlacement) == false) {
		return false;
	}

	in->mPosition = legacyPlacement;
	in->mIsLoaded = true;
	in->mPlacements[in->mCurrentMonitorConfiguration] = legacyPlacement;
	return true;
}

/**
  モニター構成変更後に該当構成の位置情報があれば復元する
  該当する位置情報がない場合は現在位置を維持する
  @return 構成に変化がない場合はUnchanged、位置を復元した場合はPlacementRestored、保存位置がない場合はNoSavedPlacement
  @param[in] hwnd 対象ウインドウハンドル
*/
WindowPosition::MonitorChangeResult WindowPosition::RestoreForMonitorChange(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE) {
		return MonitorChangeResult::Unchanged;
	}

	std::vector<RECT> rects;
	MonitorEnumerationData data{&rects};
	EnumDisplayMonitors(nullptr, nullptr, EnumerateMonitorRectangles, reinterpret_cast<LPARAM>(&data));
	std::wstring currentConfiguration = (LPCTSTR)CreateMonitorConfigurationIdentifier(rects);
	if (currentConfiguration == in->mCurrentMonitorConfiguration) {
		return MonitorChangeResult::Unchanged;
	}

	if (in->mIsLoaded && in->mCurrentMonitorConfiguration.empty() == false) {
		in->mPlacements[in->mCurrentMonitorConfiguration] = in->mPosition;
	}
	in->mCurrentMonitorConfiguration = currentConfiguration;

	auto placement = in->mPlacements.find(currentConfiguration);
	if (placement != in->mPlacements.end() && ApplyPlacement(hwnd, placement->second)) {
		in->mPosition = placement->second;
		in->mIsLoaded = true;
		return MonitorChangeResult::PlacementRestored;
	}

	WINDOWPLACEMENT currentPlacement{};
	currentPlacement.length = sizeof(WINDOWPLACEMENT);
	if (GetWindowPlacement(hwnd, &currentPlacement)) {
		in->mPosition = currentPlacement;
		in->mIsLoaded = true;
	}
	return MonitorChangeResult::NoSavedPlacement;
}

/**
  現在のウインドウ位置を保持する
  @return true:更新した false:更新しなかった
  @param[in] hwnd 対象ウインドウハンドル
*/
bool WindowPosition::Update(HWND hwnd)
{
	if (IsWindow(hwnd) == FALSE || IsCurrentMonitorConfiguration() == false) {
		return false;
	}

	WINDOWPLACEMENT wp{};
	wp.length = sizeof(wp);
	if (GetWindowPlacement(hwnd, &wp) == FALSE) {
		return false;
	}
	in->mPosition = wp;
	in->mIsLoaded = true;
	return true;
}

/**
  保持しているウインドウ位置をYAML形式で保存する
  @return true:保存した false:保存しなかった
*/
bool WindowPosition::Save()
{
	if (in->mIsLoaded == false) {
		return false;
	}

	try {
		if (in->mCurrentMonitorConfiguration.empty()) {
			std::vector<RECT> rects;
			MonitorEnumerationData data{&rects};
			EnumDisplayMonitors(nullptr, nullptr, EnumerateMonitorRectangles, reinterpret_cast<LPARAM>(&data));
			in->mCurrentMonitorConfiguration = (LPCTSTR)CreateMonitorConfigurationIdentifier(rects);
		}
		in->mPosition.length = sizeof(WINDOWPLACEMENT);
		in->mPlacements[in->mCurrentMonitorConfiguration] = in->mPosition;
		return WritePlacementYaml(GetWindowPlacementFilePath(), in->mPlacements);
	}
	catch (...) {
		return false;
	}
}

WINDOWPLACEMENT WindowPosition::GetPosition() const
{
	return in->mPosition;
}

void WindowPosition::SetPosition(const WINDOWPLACEMENT& position)
{
	in->mPosition = position;
}

bool WindowPosition::IsPositionLoaded() const
{
	return in->mIsLoaded;
}

CString WindowPosition::CreateMonitorConfigurationIdentifier(const std::vector<RECT>& monitors)
{
	std::vector<RECT> sortedMonitors(monitors);
	std::stable_sort(sortedMonitors.begin(), sortedMonitors.end(), [](const RECT& lhs, const RECT& rhs) {
		if (lhs.left != rhs.left) {
			return lhs.left < rhs.left;
		}
		if (lhs.top != rhs.top) {
			return lhs.top < rhs.top;
		}
		if (lhs.right != rhs.right) {
			return lhs.right < rhs.right;
		}
		return lhs.bottom < rhs.bottom;
	});

	std::string serialized;
	for (const auto& rect : sortedMonitors) {
		serialized += std::to_string(rect.left);
		serialized += ',';
		serialized += std::to_string(rect.top);
		serialized += ',';
		serialized += std::to_string(rect.right);
		serialized += ',';
		serialized += std::to_string(rect.bottom);
		serialized += ';';
	}

	std::vector<uint8_t> bytes(serialized.begin(), serialized.end());
	SHA1 sha;
	sha.Add(bytes);
	return sha.Finish(true);
}

bool WindowPosition::IsValidWindowPlacementData(const std::vector<uint8_t>& data, WINDOWPLACEMENT& placement)
{
	if (data.size() != sizeof(WINDOWPLACEMENT)) {
		return false;
	}
	memcpy(&placement, data.data(), sizeof(WINDOWPLACEMENT));
	return placement.length == sizeof(WINDOWPLACEMENT);
}

bool WindowPosition::IsCurrentMonitorConfiguration() const
{
	if (in->mCurrentMonitorConfiguration.empty()) {
		return true;
	}

	std::vector<RECT> rects;
	MonitorEnumerationData data{&rects};
	EnumDisplayMonitors(nullptr, nullptr, EnumerateMonitorRectangles, reinterpret_cast<LPARAM>(&data));
	return in->mCurrentMonitorConfiguration == (LPCTSTR)CreateMonitorConfigurationIdentifier(rects);
}

void WindowPosition::GetFilePath(LPCTSTR baseName, Path& path)
{
	CAppProfile::GetDirPath(path, path.size(), true);
	path.Append(baseName);
	path.AddExtension(_T(".position"));
}
