#include "pch.h"
#include "WindowPosition.h"
#include "WindowPlacementYaml.h"
#include "utility/AppProfile.h"
#include "utility/Path.h"
#include "utility/SHA1.h"
#include "app/AppName.h"
#include <algorithm>
#include <map>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

using PlacementMap = WindowPlacementYaml::PlacementMap;

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

/**
  位置情報ファイルを読み込み、ウインドウ種別ごとの位置情報を取得する
  @return true:読み込んだ false:読み込めなかった
  @param[in] filePath 位置情報ファイルのパス
  @param[out] placements 読み込んだ位置情報
*/
bool ReadPlacementYaml(LPCTSTR filePath, WindowPlacementYaml::WindowPlacementMap& placements)
{
	try {
		CFile file;
		if (file.Open(filePath, CFile::modeRead | CFile::shareDenyWrite) == FALSE) {
			return false;
		}

		ULONGLONG fileSize = file.GetLength();
		if (fileSize == 0) {
			return false;
		}

		std::string yaml(static_cast<size_t>(fileSize), '\0');
		if (file.Read(yaml.data(), static_cast<UINT>(yaml.size())) != yaml.size()) {
			return false;
		}

		return WindowPlacementYaml::Parse(yaml, placements);
	}
	catch (...) {
		placements.clear();
		return false;
	}
}

/**
  ウインドウ種別ごとの位置情報をファイルへ書き込む
  @return true:書き込んだ false:書き込めなかった
  @param[in] filePath 位置情報ファイルのパス
  @param[in] placements 書き込む位置情報
*/
bool WritePlacementYaml(LPCTSTR filePath, const WindowPlacementYaml::WindowPlacementMap& placements)
{
	try {
		auto yaml = WindowPlacementYaml::Emit(placements);
		if (yaml.empty()) {
			return false;
		}

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
	std::wstring mName;
	WINDOWPLACEMENT mPosition{};
	bool mIsLoaded{false};
	PlacementMap mPlacements;
	std::wstring mCurrentMonitorConfiguration;
};

WindowPosition::WindowPosition() : in(std::make_unique<PImpl>())
{
	in->mName = WindowPlacementYaml::GetMainWindowName();
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

	// 位置情報ファイルを読み込み、自分に該当するウインドウ種別の位置情報を抽出する
	// (他のウインドウ種別の位置情報はここでは読み込まない)
	WindowPlacementYaml::WindowPlacementMap allPlacements;
	if (ReadPlacementYaml(GetWindowPlacementFilePath(), allPlacements)) {
		auto windowEntry = allPlacements.find(in->mName);
		if (windowEntry != allPlacements.end()) {
			in->mPlacements = windowEntry->second;
		}

		auto current = in->mPlacements.find(in->mCurrentMonitorConfiguration);
		if (current != in->mPlacements.end() && ApplyPlacement(hwnd, current->second)) {
			in->mPosition = current->second;
			in->mIsLoaded = true;
			return true;
		}

		return false;
	}

	// 位置情報ファイルがない(または読み込みに失敗した)場合は、旧形式のファイルから復元する
	Path legacyPath;
	GetFilePath((LPCTSTR)in->mName.c_str(), legacyPath);
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
  他のウインドウ種別の位置情報を失わないよう、保存直前にファイルを読み直して統合する
  @return true:保存した false:保存しなかった
*/
bool WindowPosition::Save()
{
	if (in->mIsLoaded == false) {
		return false;
	}

	try {
		if (WindowPlacementYaml::IsValidWindowName(in->mName) == false) {
			// ウインドウ名をキーとして表現できない場合は保存しない
			return false;
		}

		if (in->mCurrentMonitorConfiguration.empty()) {
			std::vector<RECT> rects;
			MonitorEnumerationData data{&rects};
			EnumDisplayMonitors(nullptr, nullptr, EnumerateMonitorRectangles, reinterpret_cast<LPARAM>(&data));
			in->mCurrentMonitorConfiguration = (LPCTSTR)CreateMonitorConfigurationIdentifier(rects);
		}
		in->mPosition.length = sizeof(WINDOWPLACEMENT);
		in->mPlacements[in->mCurrentMonitorConfiguration] = in->mPosition;

		// 他ウインドウ種別の位置情報を保つため、ファイルを読み直した上で自分の分だけ差し替える
		WindowPlacementYaml::WindowPlacementMap allPlacements;
		ReadPlacementYaml(GetWindowPlacementFilePath(), allPlacements);
		allPlacements[in->mName] = in->mPlacements;

		return WritePlacementYaml(GetWindowPlacementFilePath(), allPlacements);
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
