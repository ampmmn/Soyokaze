#include "MonitorController.h"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <charconv>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

using json = nlohmann::json;
constexpr const char* kVersion = "0.0.1";

struct QueryOptions
{
	bool source = false;
	bool brightness = false;
	bool jsonOutput = false;
};

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

bool ParseInteger(const std::string& value, int& result)
{
	if (value.empty()) {
		return false;
	}
	int parsed = 0;
	auto converted = std::from_chars(value.data(), value.data() + value.size(), parsed, 10);
	if (converted.ec != std::errc() || converted.ptr != value.data() + value.size()) {
		return false;
	}
	result = parsed;
	return true;
}

void PrintGlobalHelp()
{
	std::cout << "usage: monit <command> [options]" << std::endl;
	std::cout << "commands:" << std::endl;
	std::cout << "  query       Display monitor information" << std::endl;
	std::cout << "  switch      Change monitor input source" << std::endl;
	std::cout << "  brightness  Change monitor brightness" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -h, --help     Show help" << std::endl;
	std::cout << "  -v, --version  Show version" << std::endl;
}

void PrintQueryHelp()
{
	std::cout << "usage: monit query <all|index> [options]" << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -s, --source      Show supported input sources" << std::endl;
	std::cout << "  -b, --brightness  Show current brightness" << std::endl;
	std::cout << "  -j, --json        Output JSON" << std::endl;
	std::cout << "  -h, --help        Show help" << std::endl;
}

void PrintSwitchHelp()
{
	std::cout << "usage: monit switch <index> <input-source> [<index> <input-source> ...]" << std::endl;
	std::cout << "Change the input source of one or more monitors." << std::endl;
	std::cout << "input-source may be a VCP value from 0 to 255 or an alias." << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -h, --help        Show help" << std::endl;
}

void PrintBrightnessHelp()
{
	std::cout << "usage: monit brightness <all|index> <value> [<index> <value> ...]" << std::endl;
	std::cout << "Change the brightness of one or more monitors." << std::endl;
	std::cout << "value must be an integer from 0 to 100." << std::endl;
	std::cout << "options:" << std::endl;
	std::cout << "  -h, --help        Show help" << std::endl;
}

/**
 * 引数にヘルプオプションが含まれているかを判定する
 *
 * @param[in] args コマンド引数
 * @return true:ヘルプオプションあり false:ヘルプオプションなし
 */
bool ContainsHelpOption(const std::vector<std::string>& args)
{
	return std::find(args.begin(), args.end(), "-h") != args.end() ||
		std::find(args.begin(), args.end(), "--help") != args.end();
}

bool SelectTarget(const std::string& target, std::size_t count, std::vector<std::size_t>& indexes)
{
	if (target == "all") {
		indexes.resize(count);
		for (std::size_t i = 0; i < count; ++i) {
			indexes[i] = i;
		}
		return true;
	}
	int index = 0;
	if (!ParseInteger(target, index) || index < 1 || static_cast<std::size_t>(index) > count) {
		return false;
	}
	indexes.push_back(static_cast<std::size_t>(index - 1));
	return true;
}

json MakeDeviceJson(std::size_t index, const MonitorDevice& device, const QueryOptions& options, bool& failed)
{
	json result;
	result["index"] = index + 1;
	result["displayName"] = ToUtf8(device.displayName);
	result["brightness"] = nullptr;
	result["inputSources"] = json::array();

	if (options.brightness) {
		BrightnessInfo brightness{};
		if (GetMonitorBrightness(device, brightness)) {
			result["brightness"] = {brightness.current, brightness.maximum};
		}
		else {
			failed = true;
		}
	}
	if (options.source) {
		std::vector<InputSourceInfo> sources;
		if (GetMonitorInputSources(device, sources)) {
			for (const auto& source : sources) {
				result["inputSources"].push_back({{"id", source.id}, {"displayName", source.displayName}});
			}
		}
		else {
			failed = true;
		}
	}
	return result;
}

int ExecuteQuery(const std::vector<std::string>& args)
{
	if (args.empty() || ContainsHelpOption(args)) {
		PrintQueryHelp();
		return args.empty() ? 2 : 0;
	}
	const std::string target = args[0];
	QueryOptions options;
	for (std::size_t i = 1; i < args.size(); ++i) {
		if (args[i] == "-s" || args[i] == "--source") {
			options.source = true;
		}
		else if (args[i] == "-b" || args[i] == "--brightness") {
			options.brightness = true;
		}
		else if (args[i] == "-j" || args[i] == "--json") {
			options.jsonOutput = true;
		}
		else if (args[i] == "-h" || args[i] == "--help") {
			PrintQueryHelp();
			return 0;
		}
		else {
			std::cerr << "unknown query option: " << args[i] << std::endl;
			return 2;
		}
	}
	if (target != "all" && (!options.source && !options.brightness)) {
		options.source = true;
		options.brightness = true;
	}

	MonitorSession session;
	std::string error;
	if (!session.Enumerate(error)) {
		std::cerr << error << std::endl;
		return 1;
	}
	const auto& devices = session.GetDevices();
	std::vector<std::size_t> indexes;
	if (!SelectTarget(target, devices.size(), indexes)) {
		std::cerr << "invalid monitor target: " << target << std::endl;
		return 2;
	}
	bool failed = false;
	json output;
	output["count"] = devices.size();
	output["devices"] = json::array();
	for (const std::size_t index : indexes) {
		output["devices"].push_back(MakeDeviceJson(index, devices[index], options, failed));
	}

	if (options.jsonOutput) {
		std::cout << output.dump(4) << std::endl;
	}
	else {
		std::cout << "Detected Monitor Count: " << devices.size() << std::endl;
		std::size_t outputIndex = 0;
		for (const std::size_t index : indexes) {
			const auto& device = output["devices"][outputIndex++];
			std::cout << "[" << device["index"].get<int>() << "] : " << device["displayName"].get<std::string>() << std::endl;
			if (options.brightness && !device["brightness"].is_null()) {
				std::cout << "  Brightness:" << std::endl;
				std::cout << "    " << device["brightness"][0] << "/" << device["brightness"][1] << std::endl;
			}
			if (options.source) {
				std::cout << "  Input Sources:" << std::endl;
				for (const auto& source : device["inputSources"]) {
					std::cout << "    " << source["id"].get<unsigned int>() << " : " << source["displayName"].get<std::string>() << std::endl;
				}
			}
		}
	}
	return failed ? 1 : 0;
}

int ExecuteSwitch(const std::vector<std::string>& args)
{
	if (ContainsHelpOption(args)) {
		PrintSwitchHelp();
		return 0;
	}
	if (args.empty() || args.size() % 2 != 0) {
		PrintSwitchHelp();
		return 2;
	}
	std::vector<std::pair<std::size_t, unsigned int>> operations;
	for (std::size_t i = 0; i < args.size(); i += 2) {
		int index = 0;
		unsigned int value = 0;
		if (!ParseInteger(args[i], index) || index < 1 || !ParseInputSourceValue(args[i + 1], value)) {
			std::cerr << "invalid switch argument" << std::endl;
			return 2;
		}
		operations.emplace_back(static_cast<std::size_t>(index - 1), value);
	}
	MonitorSession session;
	std::string error;
	if (!session.Enumerate(error)) {
		std::cerr << error << std::endl;
		return 1;
	}
	const auto& devices = session.GetDevices();
	bool failed = false;
	for (const auto& operation : operations) {
		if (operation.first >= devices.size()) {
			std::cerr << "monitor index out of range: " << operation.first + 1 << std::endl;
			failed = true;
			continue;
		}
		if (!SetMonitorInputSource(devices[operation.first], operation.second)) {
			std::cerr << "failed to switch monitor " << operation.first + 1 << std::endl;
			failed = true;
		}
	}
	return failed ? 1 : 0;
}

int ExecuteBrightness(const std::vector<std::string>& args)
{
	if (ContainsHelpOption(args)) {
		PrintBrightnessHelp();
		return 0;
	}
	if (args.empty() || args.size() % 2 != 0) {
		PrintBrightnessHelp();
		return 2;
	}
	std::vector<std::pair<std::string, int>> operations;
	bool hasAll = false;
	for (std::size_t i = 0; i < args.size(); i += 2) {
		int value = 0;
		if (!ParseInteger(args[i + 1], value) || value < 0 || value > 100) {
			std::cerr << "brightness must be an integer from 0 to 100" << std::endl;
			return 2;
		}
		if (args[i] == "all") {
			hasAll = true;
		}
		else {
			int index = 0;
			if (!ParseInteger(args[i], index) || index < 1) {
				std::cerr << "invalid monitor target: " << args[i] << std::endl;
				return 2;
			}
		}
		operations.emplace_back(args[i], value);
	}
	if (hasAll && operations.size() != 1) {
		std::cerr << "all cannot be combined with another target" << std::endl;
		return 2;
	}

	MonitorSession session;
	std::string error;
	if (!session.Enumerate(error)) {
		std::cerr << error << std::endl;
		return 1;
	}
	const auto& devices = session.GetDevices();
	bool failed = false;
	for (const auto& operation : operations) {
		std::vector<std::size_t> indexes;
		if (!SelectTarget(operation.first, devices.size(), indexes)) {
			std::cerr << "invalid monitor target: " << operation.first << std::endl;
			failed = true;
			continue;
		}
		for (const std::size_t index : indexes) {
			if (!SetMonitorBrightness(devices[index], operation.second)) {
				std::cerr << "failed to set brightness for monitor " << index + 1 << std::endl;
				failed = true;
			}
		}
	}
	return failed ? 1 : 0;
}

} // 名前空間

int __cdecl main(int argc, char** argv)
{
	if (argc <= 1) {
		PrintGlobalHelp();
		return 0;
	}
	std::vector<std::string> arguments(argv + 1, argv + argc);
	if (std::find(arguments.begin(), arguments.end(), "-v") != arguments.end() ||
		std::find(arguments.begin(), arguments.end(), "--version") != arguments.end()) {
		std::cout << "monit " << kVersion << std::endl;
		return 0;
	}
	if (arguments[0] == "-h" || arguments[0] == "--help") {
		PrintGlobalHelp();
		return 0;
	}

	const std::string command = arguments.front();
	arguments.erase(arguments.begin());
	if (command == "query") {
		return ExecuteQuery(arguments);
	}
	if (command == "switch") {
		return ExecuteSwitch(arguments);
	}
	if (command == "brightness") {
		return ExecuteBrightness(arguments);
	}
	std::cerr << "unknown command: " << command << std::endl;
	PrintGlobalHelp();
	return 2;
}
