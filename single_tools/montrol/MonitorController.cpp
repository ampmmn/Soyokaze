#include "MonitorController.h"

#include <algorithm>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace {

struct EnumerationContext
{
	std::vector<MonitorDevice>* devices;
};

/**
 * UTF-16文字列をログ出力用のUTF-8文字列へ変換する
 *
 * @param[in] value 変換対象の文字列
 * @return 変換後の文字列。変換できない場合は空文字列
 */
std::string ToUtf8(const std::wstring& value)
{
	if (value.empty()) {
		return {};
	}
	const int length = WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
	if (length <= 0) {
		return {};
	}
	std::string result(length, '\0');
	WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), length, nullptr, nullptr);
	return result;
}

/**
 * Windows APIの失敗情報をデバッグ出力へ記録する
 *
 * @param[in] apiName 失敗したAPI名
 * @param[in] device 対象モニター
 * @param[in] errorCode API直後に取得したエラーコード
 * @param[in] vcpCode VCPコード。指定しない場合はnullopt
 */
void LogApiFailure(const char* apiName, const MonitorDevice& device, DWORD errorCode, std::optional<BYTE> vcpCode = std::nullopt)
{
	char message[512] = {};
	const DWORD messageLength = FormatMessageA(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		nullptr,
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		message,
		static_cast<DWORD>(sizeof(message)),
		nullptr);
	if (messageLength == 0) {
		strcpy_s(message, sizeof(message), "Unknown error");
	}

	std::ostringstream stream;
	stream << "montrol: " << apiName << " failed\n"
		<< "  monitor: " << ToUtf8(device.displayName) << "\n";
	if (vcpCode.has_value()) {
		stream << "  vcp: 0x" << std::uppercase << std::hex << std::setw(2) << std::setfill('0')
			<< static_cast<unsigned int>(vcpCode.value()) << std::dec << "\n";
	}
	stream << "  error: 0x" << std::uppercase << std::hex << std::setw(8) << std::setfill('0') << errorCode
		<< std::dec << " (" << errorCode << ")\n"
		<< "  message: " << message;
	const std::string output = stream.str();
	OutputDebugStringA(output.c_str());
	OutputDebugStringA("\n");
	std::cerr << output << std::endl;
}

/**
 * 論理モニターに対応する物理モニターを列挙するコールバック
 *
 * @param[in] monitor 列挙中の論理モニター
 * @param[in] parameter EnumerationContextへのポインター
 * @return 列挙継続を示すTRUE
 */
BOOL CALLBACK EnumerateMonitorCallback(HMONITOR monitor, HDC, LPRECT, LPARAM parameter)
{
	auto* context = reinterpret_cast<EnumerationContext*>(parameter);
	DWORD count = 0;
	// 論理モニターからDDC/CI操作用の物理モニター数を取得する。
	const BOOL countResult = GetNumberOfPhysicalMonitorsFromHMONITOR(monitor, &count);
	const DWORD countError = GetLastError();
	if (!countResult || count == 0) {
		std::cerr << "montrol: GetNumberOfPhysicalMonitorsFromHMONITOR failed"
			<< " (result=" << (countResult ? 1 : 0)
			<< ", count=" << count
			<< ", error=0x" << std::hex << countError << std::dec << ")" << std::endl;
		context->devices->push_back({nullptr, {}, false});
		return TRUE;
	}

	// 取得したハンドルはセッション終了時にDestroyPhysicalMonitorsで解放する。
	std::vector<PHYSICAL_MONITOR> physicalMonitors(count);
	const BOOL physicalResult = GetPhysicalMonitorsFromHMONITOR(monitor, count, physicalMonitors.data());
	const DWORD physicalError = GetLastError();
	if (!physicalResult) {
		std::cerr << "montrol: GetPhysicalMonitorsFromHMONITOR failed"
			<< " (count=" << count
			<< ", error=0x" << std::hex << physicalError << std::dec << ")" << std::endl;
		context->devices->push_back({nullptr, {}, false});
		return TRUE;
	}
	for (const auto& physical : physicalMonitors) {
		context->devices->push_back({physical.hPhysicalMonitor, physical.szPhysicalMonitorDescription, true});
	}
	return TRUE;
}

/**
 * モニターのMCCSケイパビリティ文字列を取得する
 *
 * @param[in] device 対象モニター
 * @param[out] capabilities 取得したケイパビリティ文字列
 * @return true:成功 false:取得失敗または操作不可
 */
bool GetCapabilities(const MonitorDevice& device, std::string& capabilities)
{
	if (!device.controllable || device.handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	DWORD length = 0;
	const BOOL lengthResult = GetCapabilitiesStringLength(device.handle, &length);
	const DWORD lengthError = GetLastError();
	if (!lengthResult) {
		LogApiFailure("GetCapabilitiesStringLength", device, lengthError);
		return false;
	}
	if (length == 0) {
		return false;
	}
	// APIが返す終端文字を格納できるよう、取得長より1バイト大きく確保する。
	std::vector<char> buffer(length + 1, '\0');
	const BOOL capabilitiesResult = CapabilitiesRequestAndCapabilitiesReply(device.handle, buffer.data(), length);
	const DWORD capabilitiesError = GetLastError();
	if (!capabilitiesResult) {
		LogApiFailure("CapabilitiesRequestAndCapabilitiesReply", device, capabilitiesError);
		return false;
	}
	capabilities.assign(buffer.data());
	return true;
}

} // 名前空間

MonitorSession::~MonitorSession()
{
	// 列挙中に取得した全物理モニターハンドルをまとめて解放する。
	std::vector<PHYSICAL_MONITOR> physicalMonitors;
	for (const auto& device : mDevices) {
		if (device.handle != nullptr) {
			physicalMonitors.push_back({device.handle, {}});
		}
	}
	if (!physicalMonitors.empty()) {
		DestroyPhysicalMonitors(static_cast<DWORD>(physicalMonitors.size()), physicalMonitors.data());
	}
}

bool MonitorSession::Enumerate(std::string& error)
{
	mDevices.clear();
	EnumerationContext context{&mDevices};
	// EnumDisplayMonitorsのコールバックで物理モニターをmDevicesへ追加する。
	if (!EnumDisplayMonitors(nullptr, nullptr, EnumerateMonitorCallback, reinterpret_cast<LPARAM>(&context))) {
		error = "EnumDisplayMonitors failed";
		return false;
	}
	return true;
}

const std::vector<MonitorDevice>& MonitorSession::GetDevices() const
{
	return mDevices;
}

bool GetMonitorBrightness(const MonitorDevice& device, BrightnessInfo& brightness)
{
	if (!device.controllable || device.handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	// MCCSのVCPコード0x10は輝度を表す。返却値を0～100へ正規化する。
	MC_VCP_CODE_TYPE type = MC_SET_PARAMETER;
	DWORD current = 0;
	DWORD maximum = 0;
	const BOOL result = GetVCPFeatureAndVCPFeatureReply(device.handle, 0x10, &type, &current, &maximum);
	const DWORD error = GetLastError();
	if (!result) {
		LogApiFailure("GetVCPFeatureAndVCPFeatureReply", device, error, 0x10);
		return false;
	}
	brightness.current = static_cast<unsigned int>(NormalizeBrightness(current, 0, maximum));
	brightness.maximum = 100;
	return true;
}

bool SetMonitorBrightness(const MonitorDevice& device, int value)
{
	if (!device.controllable || device.handle == INVALID_HANDLE_VALUE) {
		return false;
	}
	// 現在の最大値を先に取得し、ツールの0～100値をモニター固有の範囲へ変換する。
	MC_VCP_CODE_TYPE type = MC_SET_PARAMETER;
	DWORD current = 0;
	DWORD maximum = 0;
	const BOOL result = GetVCPFeatureAndVCPFeatureReply(device.handle, 0x10, &type, &current, &maximum);
	const DWORD error = GetLastError();
	if (!result) {
		LogApiFailure("GetVCPFeatureAndVCPFeatureReply", device, error, 0x10);
		return false;
	}
	unsigned int deviceValue = 0;
	if (!DenormalizeBrightness(value, 0, maximum, deviceValue)) {
		return false;
	}
	const BOOL setResult = SetVCPFeature(device.handle, 0x10, deviceValue);
	const DWORD setError = GetLastError();
	if (!setResult) {
		LogApiFailure("SetVCPFeature", device, setError, 0x10);
		return false;
	}
	return true;
}

bool GetMonitorInputSources(const MonitorDevice& device, std::vector<InputSourceInfo>& sources)
{
	std::string capabilities;
	if (!GetCapabilities(device, capabilities)) {
		return false;
	}
	// VCPコード0x60の値をケイパビリティ文字列から抽出する。
	sources = ParseInputSources(capabilities);
	return true;
}

bool SetMonitorInputSource(const MonitorDevice& device, unsigned int value)
{
	if (!device.controllable || device.handle == INVALID_HANDLE_VALUE || value > 255) {
		return false;
	}
	// MCCSのVCPコード0x60へ入力ソースの値を設定する。
	const BOOL result = SetVCPFeature(device.handle, 0x60, value);
	const DWORD error = GetLastError();
	if (!result) {
		LogApiFailure("SetVCPFeature", device, error, 0x60);
		return false;
	}
	return true;
}
