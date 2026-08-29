#include "pch.h"
#include "WorkspacePlugin.h"

#define C4_EXCEPTIONS
#define RYML_DEFAULT_CALLBACK_USES_EXCEPTIONS
#define RYML_SINGLE_HDR_DEFINE_NOW
#include "rapidyaml/rapidyaml.hpp"
#include <soyokaze/PluginExportTable.h>
#include <shellapi.h>
#pragma comment(lib, "Shell32.lib")

#include <algorithm>
#include <atomic>
#include <charconv>
#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <limits>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

extern HMODULE g_hModule;

namespace {

namespace ryml = c4::yml;

struct Command { std::vector<std::string> folder; std::map<std::string, std::vector<std::string>> file; };
struct Workspace {
	std::vector<std::filesystem::path> roots;
	int limit{(std::numeric_limits<int>::max)()};
	int maxDepth{-1};
	std::string trigger;
	std::vector<std::string> extensions;
	std::vector<std::string> excludeFiles;
	std::vector<std::string> excludeDirectories;
	Command command;
};
struct Entry {
	std::string name;
	std::string path;
	bool directory{false};
	const Command* command{nullptr};
	HICON icon{nullptr};
};
struct Match { std::vector<int> indexes; std::vector<int> levels; std::string error; std::mutex mutex; };

std::mutex gMutex;
LAUNCHER_FUNCTION_TABLE gFunctions{};
std::vector<Workspace> gWorkspaces;
std::vector<Entry> gEntries;
std::thread gIndexThread;
std::atomic<bool> gInitialized{false};
std::atomic<bool> gReady{false};

void LogMessage(LPFUNCPRINTMSG function, const char* format, va_list args)
{
	if (!function || !format) return;
	va_list lengthArgs;
	va_copy(lengthArgs, args);
	const int length = std::vsnprintf(nullptr, 0, format, lengthArgs);
	va_end(lengthArgs);
	if (length < 0) return;
	std::vector<char> message(static_cast<size_t>(length) + 1);
	std::vsnprintf(message.data(), message.size(), format, args);
	function(message.data());
}

void Log(LPFUNCPRINTMSG LAUNCHER_FUNCTION_TABLE::*member, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	LPFUNCPRINTMSG function = nullptr;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		function = gFunctions.*member;
	}
	LogMessage(function, format, args);
	va_end(args);
}

std::wstring ToWide(const std::string& value)
{
	if (value.empty()) return {};
	const int length = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), nullptr, 0);
	if (length <= 0) return {};
	std::wstring result(static_cast<size_t>(length), L'\0');
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), result.data(), length);
	return result;
}

std::string ToUtf8(const std::wstring& value)
{
	if (value.empty()) return {};
	const int length = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
	if (length <= 0) return {};
	std::string result(static_cast<size_t>(length), '\0');
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), result.data(), length, nullptr, nullptr);
	return result;
}

std::string Lower(std::string value)
{
	std::transform(value.begin(), value.end(), value.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return value;
}

bool Contains(const std::string& value, const std::vector<std::string>& patterns)
{
	for (const auto& pattern : patterns) if (value.find(pattern) != std::string::npos) return true;
	return false;
}

std::string ToString(c4::csubstr value)
{
	return value.str ? std::string(value.str, value.len) : std::string();
}

bool ReadString(const ryml::ConstNodeRef& node, std::string& result)
{
	if (!node.readable() || !node.has_val() || node.val_is_null()) return false;
	result = ToString(node.val());
	return true;
}

bool ReadInteger(const ryml::ConstNodeRef& node, int& result)
{
	if (!node.readable() || !node.has_val() || node.val_is_null()) return false;
	const c4::csubstr value = node.val();
	const auto parsed = std::from_chars(value.str, value.str + value.len, result);
	return parsed.ec == std::errc() && parsed.ptr == value.str + value.len;
}

bool ReadArray(const ryml::ConstNodeRef& object, const char* key, std::vector<std::string>& result,
	bool required, bool ignoreNull = false)
{
	if (!object.readable() || !object.is_map()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid setting '%s': parent must be a mapping.", key);
		return false;
	}
	if (!object.has_child(c4::to_csubstr(key))) {
		if (required) Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid setting '%s': required key is missing.", key);
		return !required;
	}
	const auto array = object[c4::to_csubstr(key)];
	if (!array.readable() || !array.is_seq()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid setting '%s': expected an array.", key);
		return false;
	}
	result.clear();
	for (ryml::id_type index = 0; index < array.num_children(); ++index) {
		const auto child = array.child(index);
		if (ignoreNull && child.is_val() && child.val_is_null()) continue;
		std::string value;
		if (!ReadString(child, value)) {
			Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
				"Invalid setting '%s[%u]': expected a non-null scalar.", key,
				static_cast<unsigned>(index));
			return false;
		}
		result.push_back(std::move(value));
	}
	return true;
}

bool ReadCommand(const ryml::ConstNodeRef& object, const char* key, std::vector<std::string>& result)
{
	if (!ReadArray(object, key, result, true)) return false;
	if (result.empty()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid command '%s': array must not be empty.", key);
		return false;
	}
	for (size_t index = 0; index < result.size(); ++index) {
		if (result[index].empty()) {
			Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
				"Invalid command '%s[%zu]': value must not be empty.", key, index);
			return false;
		}
	}
	return true;
}

bool ReadCommandNode(const ryml::ConstNodeRef& node, const char* key, std::vector<std::string>& result)
{
	if (!node.readable() || !node.is_seq()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid command '%s': expected an array.", key);
		return false;
	}
	result.clear();
	for (ryml::id_type index = 0; index < node.num_children(); ++index) {
		std::string value;
		if (!ReadString(node.child(index), value)) {
			Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
				"Invalid command '%s[%u]': expected a non-null scalar.", key,
				static_cast<unsigned>(index));
			return false;
		}
		result.push_back(std::move(value));
	}
	if (result.empty()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid command '%s': array must not be empty.", key);
		return false;
	}
	for (size_t index = 0; index < result.size(); ++index) {
		if (result[index].empty()) {
			Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
				"Invalid command '%s[%zu]': value must not be empty.", key, index);
			return false;
		}
	}
	return true;
}

bool ReadFileCommands(const ryml::ConstNodeRef& object, const char* key,
	std::map<std::string, std::vector<std::string>>& result)
{
	if (!object.readable() || !object.is_map()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid setting '%s': parent must be a mapping.", key);
		return false;
	}
	if (!object.has_child(c4::to_csubstr(key))) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid setting '%s': required key is missing.", key);
		return false;
	}
	const auto commands = object[c4::to_csubstr(key)];
	if (!commands.readable() || !commands.is_map()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid setting '%s': expected a mapping.", key);
		return false;
	}
	result.clear();
	for (ryml::id_type index = 0; index < commands.num_children(); ++index) {
		const auto command = commands.child(index);
		std::string extension = Lower(ToString(command.key()));
		if (extension.empty() || (extension != "default" && (extension.front() != '.' || extension.size() == 1))) {
			Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
				"Invalid setting '%s': key must be an extension beginning with '.' or 'default'.", key);
			return false;
		}
		if (result.contains(extension)) {
			Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
				"Invalid setting '%s': duplicate key '%s'.", key, extension.c_str());
			return false;
		}
		std::vector<std::string> arguments;
		if (!ReadCommandNode(command, extension.c_str(), arguments)) return false;
		result.emplace(std::move(extension), std::move(arguments));
	}
	if (result.empty()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Invalid setting '%s': mapping must not be empty.", key);
		return false;
	}
	return true;
}

bool ReadWorkspace(const ryml::ConstNodeRef& object, const std::filesystem::path& base,
	size_t index, Workspace& result)
{
	if (!object.readable() || !object.is_map()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu]: expected a mapping.", index);
		return false;
	}
	std::vector<std::string> roots;
	if (!ReadArray(object, "directory", roots, true)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].directory.", index);
		return false;
	}
	if (roots.empty()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].directory: array must not be empty.", index);
		return false;
	}
	if (!object.has_child(c4::to_csubstr("execute"))) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu]: required key 'execute' is missing.", index);
		return false;
	}
	const auto execute = object[c4::to_csubstr("execute")];
	if (!execute.readable() || !execute.is_map()) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].execute: expected a mapping.", index);
		return false;
	}
	if (!ReadCommand(execute, "folder", result.command.folder)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].execute.folder.", index);
		return false;
	}
	if (!ReadFileCommands(execute, "file", result.command.file)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].execute.file.", index);
		return false;
	}
	for (const auto& root : roots) {
		std::filesystem::path path(ToWide(root));
		if (path.is_relative()) path = base / path;
		result.roots.push_back(std::filesystem::absolute(path).lexically_normal());
	}
	const auto searchLimitKey = c4::to_csubstr("search-limit");
	if (object.has_child(searchLimitKey) && !ReadInteger(object[searchLimitKey], result.limit)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].search-limit: expected an integer.", index);
		return false;
	}
	if (result.limit <= 0) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].search-limit: value must be positive.", index);
		return false;
	}
	const auto maxDepthKey = c4::to_csubstr("max-depth");
	if (object.has_child(maxDepthKey) && !ReadInteger(object[maxDepthKey], result.maxDepth)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].max-depth: expected an integer.", index);
		return false;
	}
	if (result.maxDepth < -1) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].max-depth: value must be -1 or greater.", index);
		return false;
	}
	const auto searchTriggerWordKey = c4::to_csubstr("search-trigger-word");
	if (object.has_child(searchTriggerWordKey) &&
		!ReadString(object[searchTriggerWordKey], result.trigger)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].search-trigger-word: expected a non-null scalar.", index);
		return false;
	}
	if (!ReadArray(object, "include-ext", result.extensions, false)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].include-ext.", index);
		return false;
	}
	if (!ReadArray(object, "exclude-file", result.excludeFiles, false, true)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].exclude-file.", index);
		return false;
	}
	if (!ReadArray(object, "exclude-dir", result.excludeDirectories, false, true)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog,
			"Invalid workspace[%zu].exclude-dir.", index);
		return false;
	}
	result.excludeFiles.erase(std::remove(result.excludeFiles.begin(), result.excludeFiles.end(), ""),
		result.excludeFiles.end());
	result.excludeDirectories.erase(
		std::remove(result.excludeDirectories.begin(), result.excludeDirectories.end(), ""),
		result.excludeDirectories.end());
	for (auto& extension : result.extensions) extension = Lower(extension);
	for (auto& pattern : result.excludeFiles) pattern = Lower(pattern);
	for (auto& pattern : result.excludeDirectories) pattern = Lower(pattern);
	return true;
}

bool LoadSettings(const std::filesystem::path& path, std::vector<Workspace>& result)
{
	std::ifstream file(path, std::ios::binary);
	if (!file) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to open settings file: %s", ToUtf8(path.wstring()).c_str());
		return false;
	}
	try {
		const std::string source((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
		const auto tree = ryml::parse_in_arena(c4::csubstr(source.data(), source.size()));
		const auto root = tree.crootref();
		if (!root.readable()) {
			Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Invalid settings: root element is not readable.");
			return false;
		}
		if (!root.is_seq()) {
			Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Invalid settings: root element must be an array.");
			return false;
		}
		const auto workspaces = root;
		for (ryml::id_type index = 0; index < workspaces.num_children(); ++index) {
			Workspace workspace;
			if (ReadWorkspace(workspaces.child(index), path.parent_path(), static_cast<size_t>(index), workspace)) {
				result.push_back(std::move(workspace));
			} else {
				Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Skipping an invalid workspace.");
			}
		}
	}
	catch (const std::exception& exception) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to parse settings: %s", exception.what());
		return false;
	}
	return true;
}

bool Excluded(const std::filesystem::path& path, bool directory, const Workspace& workspace)
{
	return Contains(Lower(ToUtf8(path.filename().wstring())),
		directory ? workspace.excludeDirectories : workspace.excludeFiles);
}

bool Included(const std::filesystem::path& path, const Workspace& workspace)
{
	if (workspace.extensions.empty()) return false;
	const std::string extension = Lower(ToUtf8(path.extension().wstring()));
	for (const auto& accepted : workspace.extensions) if (accepted == "*" || accepted == extension) return true;
	return false;
}

void AddEntry(const std::filesystem::path& path, const std::filesystem::path& root,
	bool directory, const Workspace& workspace, std::vector<Entry>& result)
{
	const std::string relative = ToUtf8(path.lexically_relative(root).wstring());
	if (relative.empty()) return;
	result.push_back({ workspace.trigger.empty() ? relative : workspace.trigger + " " + relative,
		ToUtf8(path.wstring()), directory, &workspace.command, nullptr });
}

void IndexRoot(const std::filesystem::path& root, const Workspace& workspace, std::vector<Entry>& result)
{
	if (!std::filesystem::is_directory(root)) {
		Log(&LAUNCHER_FUNCTION_TABLE::WarnLog, "Skipping directory: %s", ToUtf8(root.wstring()).c_str());
		return;
	}
	std::error_code error;
	std::filesystem::recursive_directory_iterator iterator(root,
		std::filesystem::directory_options::skip_permission_denied, error);
	const std::filesystem::recursive_directory_iterator end;
	while (iterator != end) {
		const auto path = iterator->path();
		const bool directory = iterator->is_directory(error);
		const int depth = static_cast<int>(iterator.depth());
		if (error) error.clear();
		if (workspace.maxDepth >= 0 && depth > workspace.maxDepth) {
			if (directory) iterator.disable_recursion_pending();
		} else if (Excluded(path, directory, workspace)) {
			if (directory) iterator.disable_recursion_pending();
		} else if (directory || Included(path, workspace)) {
			AddEntry(path, root, directory, workspace, result);
		}
		iterator.increment(error);
	}
}

void BuildIndex(std::filesystem::path settingsPath)
{
	std::vector<Workspace> workspaces;
	std::vector<Entry> entries;
	try {
		if (LoadSettings(settingsPath, workspaces)) {
			for (const auto& workspace : workspaces)
				for (const auto& root : workspace.roots) IndexRoot(root, workspace, entries);
		}
	}
	catch (const std::exception& exception) {
		Log(&LAUNCHER_FUNCTION_TABLE::ErrorLog, "Failed to build workspace index: %s", exception.what());
		workspaces.clear();
		entries.clear();
	}
	{
		std::lock_guard<std::mutex> lock(gMutex);
		gWorkspaces = std::move(workspaces);
		gEntries = std::move(entries);
	}
	gReady = true;
	Log(&LAUNCHER_FUNCTION_TABLE::InfoLog, "Workspace index is ready: %zu entries.", gEntries.size());
}

bool GetEntry(LNCRPLUGINMATCHHANDLE handle, int index, const Entry*& entry)
{
	if (!handle) return false;
	const auto* match = static_cast<const Match*>(handle);
	if (index < 0 || index >= static_cast<int>(match->indexes.size())) return false;
	std::lock_guard<std::mutex> lock(gMutex);
	const int entryIndex = match->indexes[index];
	if (entryIndex < 0 || entryIndex >= static_cast<int>(gEntries.size())) return false;
	entry = &gEntries[entryIndex];
	return true;
}

int CopyString(const std::string& value, char* buffer, size_t length)
{
	if (length == 0) return static_cast<int>(value.size() + 1);
	if (!buffer) return -1;
	const size_t copied = (std::min)(length, value.size() + 1);
	memcpy(buffer, value.c_str(), copied);
	buffer[copied - 1] = '\0';
	return static_cast<int>(copied);
}

std::string ReplacePath(std::string value, const std::string& path)
{
	size_t position = 0;
	while ((position = value.find("{path}", position)) != std::string::npos) {
		value.replace(position, 6, path);
		position += path.size();
	}
	return value;
}

std::wstring Quote(const std::wstring& value)
{
	if (!value.empty() && value.find_first_of(L" \t\"") == std::wstring::npos) return value;
	std::wstring result = L"\"";
	size_t slashes = 0;
	for (const wchar_t character : value) {
		if (character == L'\\') ++slashes;
		else if (character == L'\"') {
			result.append(slashes * 2 + 1, L'\\'); result += character; slashes = 0;
		} else {
			result.append(slashes, L'\\'); result += character; slashes = 0;
		}
	}
	result.append(slashes * 2, L'\\');
	return result + L'\"';
}

int Execute(LNCRPLUGINMATCHHANDLE handle, int index, int argc, char** argv)
{
	UNREFERENCED_PARAMETER(argc); UNREFERENCED_PARAMETER(argv);
	const Entry* entry = nullptr;
	if (!GetEntry(handle, index, entry)) return 1;
	if (!entry->directory) {
		const auto extension = Lower(ToUtf8(std::filesystem::path(ToWide(entry->path)).extension().wstring()));
		auto command = entry->command->file.find(extension);
		if (command == entry->command->file.end()) command = entry->command->file.find("default");
		if (command == entry->command->file.end()) {
			SHELLEXECUTEINFOW shellExecute{};
			shellExecute.cbSize = sizeof(shellExecute);
			shellExecute.fMask = SEE_MASK_FLAG_NO_UI;
			shellExecute.lpVerb = L"open";
			const std::wstring path = ToWide(entry->path);
			shellExecute.lpFile = path.c_str();
			if (!ShellExecuteExW(&shellExecute)) {
				std::lock_guard<std::mutex> lock(static_cast<Match*>(handle)->mutex);
				static_cast<Match*>(handle)->error = "Failed to open file with the associated application.";
				return 1;
			}
			if (shellExecute.hProcess) CloseHandle(shellExecute.hProcess);
			return 0;
		}
		const auto& arguments = command->second;
		std::wstring commandLine;
		for (const auto& argument : arguments) {
			if (!commandLine.empty()) commandLine += L' ';
			commandLine += Quote(ToWide(ReplacePath(argument, entry->path)));
		}
		std::vector<wchar_t> commandLineBuffer(commandLine.begin(), commandLine.end());
		commandLineBuffer.push_back(L'\0');
		STARTUPINFOW startup{}; startup.cb = sizeof(startup);
		PROCESS_INFORMATION process{};
		if (!CreateProcessW(nullptr, commandLineBuffer.data(), nullptr, nullptr, FALSE,
			CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr, &startup, &process)) {
			std::lock_guard<std::mutex> lock(static_cast<Match*>(handle)->mutex);
			static_cast<Match*>(handle)->error = "Failed to execute command.";
			return 1;
		}
		CloseHandle(process.hThread); CloseHandle(process.hProcess);
		return 0;
	}
	const auto& arguments = entry->command->folder;
	std::wstring commandLine;
	for (const auto& argument : arguments) {
		if (!commandLine.empty()) commandLine += L' ';
		commandLine += Quote(ToWide(ReplacePath(argument, entry->path)));
	}
	std::vector<wchar_t> command(commandLine.begin(), commandLine.end()); command.push_back(L'\0');
	STARTUPINFOW startup{}; startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	if (!CreateProcessW(nullptr, command.data(), nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT,
		nullptr, nullptr, &startup, &process)) {
		std::lock_guard<std::mutex> lock(static_cast<Match*>(handle)->mutex);
		static_cast<Match*>(handle)->error = "Failed to execute command.";
		return 1;
	}
	CloseHandle(process.hThread); CloseHandle(process.hProcess);
	return 0;
}

int GetIcon(LNCRPLUGINMATCHHANDLE handle, int index, HICON* icon)
{
	if (!icon) return 1;
	const Entry* entry = nullptr;
	if (!GetEntry(handle, index, entry)) return 1;
	std::lock_guard<std::mutex> lock(gMutex);
	if (entry->icon && gFunctions.HasIcon && gFunctions.HasIcon(entry->icon)) {
		*icon = entry->icon;
		return 0;
	}
	if (!gFunctions.LoadIconFromPath) return 1;
	const_cast<Entry*>(entry)->icon = gFunctions.LoadIconFromPath(entry->path.c_str());
	*icon = entry->icon;
	return entry->icon ? 0 : 1;
}

int InitializeApi(LAUNCHER_FUNCTION_TABLE* table, const char** pluginInfo)
{
	if (!table || !pluginInfo) return 1;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		gFunctions = *table;
	}
	// プラグインのメタデータはDLLのアンロードまで有効な静的文字列で返す。
	static constexpr char info[] = R"json({
  "displayName": "workspace-plugin",
  "pluginId": "A555448F-0F0F-429B-B783-CB2D4A6CA2D9",
  "pluginVersion": "0.1.0",
  "pluginApiVersion": 102,
  "pluginDescription": "指定ディレクトリ以下のファイルやディレクトリ要素を検索する",
  "pluginDeveloper": "ampmmn",
  "pluginLicenseName": "MIT License",
  "url": "https://github.com/ampmmn/Soyokaze/tree/main/plugins/workspace-plugin"
})json";
	*pluginInfo = info;
	HMODULE module = nullptr;
	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCWSTR>(&LNCRPLUGIN_Bind), &module)) return 1;
	wchar_t modulePath[MAX_PATH]{};
	const DWORD length = GetModuleFileNameW(module, modulePath, ARRAYSIZE(modulePath));
	if (!length || length == ARRAYSIZE(modulePath)) return 1;
	gReady = false; gInitialized = true;
	gIndexThread = std::thread(BuildIndex, std::filesystem::path(modulePath).parent_path() / L"settings.yaml");
	return 0;
}

LNCRPLUGINMATCHHANDLE Query(void* context, MATCHER_FUNCTION_TABLE* table)
{
	if (!context || !table || !table->Match || !table->GetFirstWord || !table->GetWordCount ||
		!gInitialized || !gReady) return nullptr;
	std::lock_guard<std::mutex> lock(gMutex);
	const char* firstWord = table->GetFirstWord(context);
	const int wordCount = table->GetWordCount(context);
	std::unique_ptr<Match> result(new Match());
	for (const auto& workspace : gWorkspaces) {
		if (!workspace.trigger.empty() && (!firstWord || workspace.trigger != firstWord)) continue;
		const int offset = workspace.trigger.empty() ? 0 : 1;
		int found = 0;
		for (int index = 0; index < static_cast<int>(gEntries.size()) && found < workspace.limit; ++index) {
			const Entry& entry = gEntries[index];
			if (entry.command != &workspace.command) continue;

			int level = table->Match(context, entry.name.c_str(), offset);
			if (level == -1) continue;

			// 検索発動ワードが設定されている場合は前方一致扱い
			if (level == 3 && offset == 1) {
			 	level = 4;
			}

			result->indexes.push_back(index); result->levels.push_back(level); ++found;
		}
	}
	return result.release();
}

void FinalizeApi()
{
	if (gIndexThread.joinable()) gIndexThread.join();
	std::lock_guard<std::mutex> lock(gMutex);
	for (auto& entry : gEntries) entry.icon = nullptr;
	gEntries.clear(); gWorkspaces.clear(); gFunctions = LAUNCHER_FUNCTION_TABLE();
	gReady = false; gInitialized = false;
}

} // 名前空間

namespace workspace {
bool Initialize() { return true; }
void Finalize() { FinalizeApi(); }
} // workspace名前空間

extern "C" int LNCRPLUGIN_API LNCRPLUGIN_Bind(int version, LNCRPLUGIN_EXPORTTABLE* table)
{
	if (version != PLUGINVERSION || !table) return 1;
	table->Initialize = &InitializeApi;
	table->Query = &Query;
	table->GetMatchCount = [](LNCRPLUGINMATCHHANDLE handle) {
		return handle ? static_cast<int>(static_cast<Match*>(handle)->indexes.size()) : 0;
	};
	table->GetMatchLevel = [](LNCRPLUGINMATCHHANDLE handle, int index) {
		if (!handle || index < 0 || index >= static_cast<int>(static_cast<Match*>(handle)->levels.size())) return -1;
		return static_cast<Match*>(handle)->levels[index];
	};
	table->CloseHandle = [](LNCRPLUGINMATCHHANDLE handle) { delete static_cast<Match*>(handle); };
	table->GetName = [](LNCRPLUGINMATCHHANDLE handle, int index, char* buffer, size_t length) {
		const Entry* entry = nullptr; return GetEntry(handle, index, entry) ? CopyString(entry->name, buffer, length) : -1;
	};
	table->GetDescription = [](LNCRPLUGINMATCHHANDLE handle, int index, char* buffer, size_t length) {
		const Entry* entry = nullptr; return GetEntry(handle, index, entry) ? CopyString(entry->path, buffer, length) : -1;
	};
	table->GetGuide = [](LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length) {
		return CopyString("Open workspace item", buffer, length);
	};
	table->GetTypeDisplayName = [](LNCRPLUGINMATCHHANDLE, int, char* buffer, size_t length) {
		return CopyString("Workspace", buffer, length);
	};
	table->CanExecute = [](LNCRPLUGINMATCHHANDLE handle, int index) {
		const Entry* entry = nullptr; return GetEntry(handle, index, entry) && entry->command ? 1 : 0;
	};
	table->Execute = &Execute;
	table->GetErrorString = [](LNCRPLUGINMATCHHANDLE handle, int index, char* buffer, size_t length) {
		const Entry* entry = nullptr; if (!GetEntry(handle, index, entry)) return -1;
		auto* match = static_cast<Match*>(handle);
		std::lock_guard<std::mutex> lock(match->mutex);
		return CopyString(match->error, buffer, length);
	};
	table->GetIcon = &GetIcon;
	table->Finalize = &FinalizeApi;
	return 0;
}
