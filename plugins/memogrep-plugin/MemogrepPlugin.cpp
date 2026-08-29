#include "pch.h"
#include "MemogrepPlugin.h"
#pragma warning(disable: 4828)
#include "resource.h"
#include <nlohmann/json.hpp>
#include <soyokaze/PluginExportTable.h>
#include <windows.h>
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <vector>

extern HMODULE g_hModule;

namespace {

using json = nlohmann::json;

struct Entry
{
	std::string filePath;
	int lineNumber{0};
	std::string date;
	std::string title;
	std::string description;
	std::vector<std::string> tags;
	std::string searchableText;
};

struct Settings
{
	std::vector<std::string> sourceFiles;
	int searchLimit{0};
	std::string triggerWord;
	std::vector<std::string> excludeTags;
	std::vector<std::string> excludeTitle;
	std::vector<std::string> excludeBody;
	std::vector<std::string> execute;
};

struct MatchHandle
{
	std::vector<int> entryIndexes;
	std::vector<int> matchLevels;
	std::string error;
	std::mutex mutex;
};

std::mutex gMutex;
Settings gSettings;
std::vector<Entry> gEntries;
LAUNCHER_FUNCTION_TABLE gLauncherFunctions{};
bool gInitialized{false};

void LogMessage(LPFUNCPRINTMSG function, const char* format, va_list arguments)
{
	if (function == nullptr || format == nullptr) {
		return;
	}

	va_list lengthArguments;
	va_copy(lengthArguments, arguments);
	const int length = std::vsnprintf(nullptr, 0, format, lengthArguments);
	va_end(lengthArguments);
	if (length < 0) {
		return;
	}

	std::vector<char> message(static_cast<size_t>(length) + 1);
	std::vsnprintf(message.data(), message.size(), format, arguments);
	function(message.data());
}

void LogInfo(const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LPFUNCPRINTMSG function = nullptr;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		function = gLauncherFunctions.InfoLog;
	}
	LogMessage(function, format, arguments);
	va_end(arguments);
}

void LogWarning(const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LPFUNCPRINTMSG function = nullptr;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		function = gLauncherFunctions.WarnLog;
	}
	LogMessage(function, format, arguments);
	va_end(arguments);
}

void LogError(const char* format, ...)
{
	va_list arguments;
	va_start(arguments, format);
	LPFUNCPRINTMSG function = nullptr;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		function = gLauncherFunctions.ErrorLog;
	}
	LogMessage(function, format, arguments);
	va_end(arguments);
}

std::wstring Utf8ToWide(const std::string& value)
{
	if (value.empty()) {
		return std::wstring();
	}
	int len = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), nullptr, 0);
	if (len <= 0) {
		return std::wstring();
	}
	std::wstring result(static_cast<size_t>(len), L'\0');
	MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), result.data(), len);
	return result;
}

std::string WideToUtf8(const std::wstring& value)
{
	if (value.empty()) {
		return std::string();
	}
	int len = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
	if (len <= 0) {
		return std::string();
	}
	std::string result(static_cast<size_t>(len), '\0');
	WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
		static_cast<int>(value.size()), result.data(), len, nullptr, nullptr);
	return result;
}

std::string Trim(const std::string& value)
{
	const auto begin = value.find_first_not_of(" \t\r");
	if (begin == std::string::npos) {
		return std::string();
	}
	const auto end = value.find_last_not_of(" \t\r");
	return value.substr(begin, end - begin + 1);
}

bool TryGetDate(const std::string& line, std::string& date)
{
	if (line.size() < 10 || line.front() == '\t' ||
		line[4] != '-' || line[7] != '-') {
		return false;
	}
	for (size_t index : { 0u, 1u, 2u, 3u, 5u, 6u, 8u, 9u }) {
		if (line[index] < '0' || line[index] > '9') {
			return false;
		}
	}
	if (line.size() > 10 && line[10] != ' ' && line[10] != '\t') {
		return false;
	}
	date = line.substr(0, 10);
	return true;
}

std::vector<std::string> SplitTags(const std::string& value)
{
	std::vector<std::string> tags;
	std::stringstream stream(value);
	std::string tag;
	while (std::getline(stream, tag, ',')) {
		tag = Trim(tag);
		if (!tag.empty()) {
			tags.push_back(tag);
		}
	}
	return tags;
}

bool ContainsAny(const std::string& text, const std::vector<std::string>& words)
{
	for (const auto& word : words) {
		if (text.find(word) != std::string::npos) {
			return true;
		}
	}
	return false;
}

bool HasExcludedTag(const std::vector<std::string>& tags,
	const std::vector<std::string>& excluded)
{
	for (const auto& tag : tags) {
		if (std::find(excluded.begin(), excluded.end(), tag) != excluded.end()) {
			return true;
		}
	}
	return false;
}

bool ReadString(const json& object, const char* key, std::string& value)
{
	const auto it = object.find(key);
	if (it == object.end() || !it->is_string()) {
		return false;
	}
	value = it->get<std::string>();
	return true;
}

bool ReadStringArray(const json& object, const char* key,
	std::vector<std::string>& values, bool required)
{
	const auto it = object.find(key);
	if (it == object.end()) {
		return !required;
	}
	if (!it->is_array()) {
		return false;
	}
	values.clear();
	for (const auto& item : *it) {
		if (!item.is_string()) {
			return false;
		}
		values.push_back(item.get<std::string>());
	}
	return true;
}

bool LoadSettings(const std::filesystem::path& settingsPath, Settings& settings)
{
	std::ifstream file(settingsPath, std::ios::binary);
	if (!file) {
		const std::string path = WideToUtf8(settingsPath.wstring());
		LogError("Failed to open settings file: %s", path.c_str());
		return false;
	}

	try {
		json object;
		file >> object;
		if (!object.is_object()) {
			const std::string path = WideToUtf8(settingsPath.wstring());
			LogError("Settings file is not a JSON object: %s", path.c_str());
			return false;
		}

		if (!ReadStringArray(object, "source-files", settings.sourceFiles, true) ||
			settings.sourceFiles.empty() ||
			!object.contains("search-limit") || !object["search-limit"].is_number_integer() ||
			!ReadString(object, "search-trigger-word", settings.triggerWord) ||
			settings.triggerWord.empty() ||
			!ReadStringArray(object, "exclude-tags", settings.excludeTags, false) ||
			!ReadStringArray(object, "exclude-title", settings.excludeTitle, false) ||
			!ReadStringArray(object, "exclude-body", settings.excludeBody, false) ||
			!ReadStringArray(object, "execute", settings.execute, true) ||
			settings.execute.empty()) {
			const std::string path = WideToUtf8(settingsPath.wstring());
			LogError("Settings file contains invalid or missing values: %s", path.c_str());
			return false;
		}

		settings.searchLimit = object["search-limit"].get<int>();
		if (settings.searchLimit <= 0) {
			LogError("The search-limit setting must be greater than zero.");
			return false;
		}
		for (const auto& argument : settings.execute) {
			if (argument.empty()) {
				LogError("The execute setting contains an empty argument.");
				return false;
			}
		}
		for (const auto& path : settings.sourceFiles) {
			if (!std::filesystem::path(Utf8ToWide(path)).is_absolute()) {
				LogError("A source file path is not absolute: %s", path.c_str());
				return false;
			}
		}
	}
	catch (const json::exception& exception) {
		const std::string path = WideToUtf8(settingsPath.wstring());
		LogError("Failed to parse settings file %s: %s", path.c_str(), exception.what());
		return false;
	}

	return true;
}

bool AddEntriesFromFile(const std::string& filePath, const Settings& settings,
	std::vector<Entry>& entries)
{
	std::ifstream file(std::filesystem::path(Utf8ToWide(filePath)), std::ios::binary);
	if (!file) {
		LogWarning("Failed to open source file: %s", filePath.c_str());
		return false;
	}

	std::string line;
	std::string body;
	std::string currentDate;
	Entry current;
	bool hasCurrent = false;
	int lineNumber = 0;

	auto finishEntry = [&]() {
		if (!hasCurrent) {
			return;
		}
		if (current.description.empty()) {
			current.description = current.title;
		}
		current.searchableText = current.title + " " + body;
		for (const auto& tag : current.tags) {
			current.searchableText += " " + tag;
		}
		if (!HasExcludedTag(current.tags, settings.excludeTags) &&
			!ContainsAny(current.title, settings.excludeTitle) &&
			!ContainsAny(body, settings.excludeBody)) {
			entries.push_back(std::move(current));
		}
		current = Entry();
		body.clear();
		hasCurrent = false;
	};

	while (std::getline(file, line)) {
		++lineNumber;
		if (!line.empty() && line.back() == '\r') {
			line.pop_back();
		}
		if (TryGetDate(line, currentDate)) {
			continue;
		}

		if (line.rfind("\t*", 0) == 0) {
			finishEntry();
			const std::string header = line.substr(2);
			const size_t separator = header.find(':');
			if (separator == std::string::npos) {
				continue;
			}
			current.lineNumber = lineNumber;
			current.filePath = filePath;
			current.date = currentDate;
			current.tags = SplitTags(header.substr(0, separator));
			current.title = Trim(header.substr(separator + 1));
			hasCurrent = true;
			continue;
		}

		if (!hasCurrent) {
			continue;
		}
		if (line.empty()) {
			finishEntry();
			continue;
		}
		if (current.description.empty()) {
			current.description = Trim(line);
		}
		if (!body.empty()) {
			body += '\n';
		}
		body += Trim(line);
	}
	finishEntry();
	return true;
}

bool LoadIndex()
{
	HMODULE module = nullptr;
	if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
		GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		reinterpret_cast<LPCWSTR>(&LNCRPLUGIN_Bind), &module)) {
		LogError("Failed to get the plugin module handle.");
		return false;
	}

	wchar_t modulePath[MAX_PATH]{};
	DWORD length = GetModuleFileNameW(module, modulePath, ARRAYSIZE(modulePath));
	if (length == 0 || length == ARRAYSIZE(modulePath)) {
		LogError("Failed to get the plugin module path.");
		return false;
	}

	Settings settings;
	const std::filesystem::path settingsPath =
		std::filesystem::path(modulePath).parent_path() / L"settings.json";
	if (!LoadSettings(settingsPath, settings)) {
		LogError("Failed to load plugin settings.");
		return false;
	}

	std::vector<Entry> entries;
	for (const auto& path : settings.sourceFiles) {
		if (!AddEntriesFromFile(path, settings, entries)) {
			LogWarning("Skipping source file: %s", path.c_str());
		}
	}
	if (entries.empty()) {
		bool anySourceExists = false;
		for (const auto& path : settings.sourceFiles) {
			if (std::filesystem::exists(std::filesystem::path(Utf8ToWide(path)))) {
				anySourceExists = true;
				break;
			}
		}
		if (!anySourceExists) {
			LogError("No configured source files could be found.");
			return false;
		}
	}

	size_t entryCount = 0;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		gSettings = std::move(settings);
		gEntries = std::move(entries);
		gInitialized = true;
		entryCount = gEntries.size();
	}
	LogInfo("Loaded %zu memo entries.", entryCount);
	return true;
}

std::string Replace(const std::string& value, const std::string& name,
	const std::string& replacement)
{
	std::string result = value;
	size_t pos = 0;
	while ((pos = result.find(name, pos)) != std::string::npos) {
		result.replace(pos, name.size(), replacement);
		pos += replacement.size();
	}
	return result;
}

std::wstring QuoteCommandArgument(const std::wstring& value)
{
	if (value.empty()) {
		return L"\"\"";
	}
	if (value.find_first_of(L" \t\"") == std::wstring::npos) {
		return value;
	}

	std::wstring result = L"\"";
	size_t backslashes = 0;
	for (wchar_t ch : value) {
		if (ch == L'\\') {
			++backslashes;
			continue;
		}
		if (ch == L'"') {
			result.append(backslashes * 2 + 1, L'\\');
			result += L'"';
			backslashes = 0;
			continue;
		}
		result.append(backslashes, L'\\');
		backslashes = 0;
		result += ch;
	}
	result.append(backslashes * 2, L'\\');
	result += L'"';
	return result;
}

int SetError(LNCRPLUGINMATCHHANDLE handle, const std::string& error)
{
	auto* match = static_cast<MatchHandle*>(handle);
	{
		std::lock_guard<std::mutex> lock(match->mutex);
		match->error = error;
	}
	LogError("%s", error.c_str());
	return 1;
}

int Match(void* context, const char* text, int offset)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(text);
	UNREFERENCED_PARAMETER(offset);
	return -1;
}

int Initialize(LAUNCHER_FUNCTION_TABLE* table, const char** pluginInfo)
{
	if (table == nullptr || pluginInfo == nullptr) {
		return 1;
	}

	{
		std::lock_guard<std::mutex> lock(gMutex);
		gLauncherFunctions = *table;
	}

	// プラグインのメタデータはDLLのアンロードまで有効な静的文字列で返す。
	static constexpr char info[] = R"json({
  "displayName": "memogrep-plugin",
  "pluginId": "F82EB950-7524-4AD4-A945-BA607E83E612",
  "pluginVersion": "0.1.0",
  "pluginApiVersion": 102,
  "pluginDescription": "ChangeLogメモをインクリメンタル検索する",
  "pluginDeveloper": "ampmmn",
  "pluginLicenseName": "MIT License",
  "url": "https://github.com/ampmmn/Soyokaze/tree/main/plugins/memogrep-plugin"
})json";
	*pluginInfo = info;

	return memogrep::Initialize() ? 0 : 1;
}

int GetMatchCount(LNCRPLUGINMATCHHANDLE handle)
{
	return static_cast<int>(static_cast<MatchHandle*>(handle)->entryIndexes.size());
}

int GetMatchLevel(LNCRPLUGINMATCHHANDLE handle, int index)
{
	auto* match = static_cast<MatchHandle*>(handle);
	if (index < 0 || index >= static_cast<int>(match->matchLevels.size())) {
		return -1;
	}
	return match->matchLevels[index];
}

void CloseMatchHandle(LNCRPLUGINMATCHHANDLE handle)
{
	delete static_cast<MatchHandle*>(handle);
}

bool GetEntry(LNCRPLUGINMATCHHANDLE handle, int index, const Entry** entry)
{
	auto* match = static_cast<MatchHandle*>(handle);
	if (index < 0 || index >= static_cast<int>(match->entryIndexes.size())) {
		return false;
	}
	std::lock_guard<std::mutex> lock(gMutex);
	const int entryIndex = match->entryIndexes[index];
	if (entryIndex < 0 || entryIndex >= static_cast<int>(gEntries.size())) {
		return false;
	}
	*entry = &gEntries[entryIndex];
	return true;
}

int GetString(LNCRPLUGINMATCHHANDLE handle, int index, char* buffer, size_t len,
	const std::string& (*getValue)(const Entry&))
{
	const Entry* entry = nullptr;
	if (!GetEntry(handle, index, &entry)) {
		return -1;
	}
	const std::string& value = getValue(*entry);
	const size_t required = value.size() + 1;
	if (len == 0) {
		return static_cast<int>(required);
	}
	const size_t copied = (std::min)(len, required);
	memcpy(buffer, value.c_str(), copied);
	buffer[copied - 1] = '\0';
	return static_cast<int>(copied);
}

void AppendNamePart(std::string& value, const std::string& part)
{
	if (part.empty()) {
		return;
	}
	if (!value.empty()) {
		value += ' ';
	}
	value += part;
}

const std::string& GetName(const Entry& entry)
{
	static thread_local std::string value;
	value.clear();
	AppendNamePart(value, entry.date);
	for (const auto& tag : entry.tags) {
		AppendNamePart(value, tag);
	}
	AppendNamePart(value, entry.title.empty() ? entry.description : entry.title);
	return value;
}

const std::string& GetDescription(const Entry& entry)
{
	return entry.description;
}

const std::string& GetGuide(const Entry& entry)
{
	UNREFERENCED_PARAMETER(entry);
	static const std::string guide = "メモをエディタで開く";
	return guide;
}

const std::string& GetTypeDisplayName(const Entry& entry)
{
	UNREFERENCED_PARAMETER(entry);
	static const std::string type = "MemoGrep";
	return type;
}

const std::string& GetErrorString(const Entry& entry)
{
	UNREFERENCED_PARAMETER(entry);
	static const std::string empty;
	return empty;
}

int QueryGetMatch(LNCRPLUGINMATCHHANDLE handle, int index, const Entry** entry)
{
	return GetEntry(handle, index, entry) ? 0 : -1;
}

LNCRPLUGINMATCHHANDLE Query(void* context, MATCHER_FUNCTION_TABLE* table)
{
	if (context == nullptr || table == nullptr) {
		return nullptr;
	}

	std::lock_guard<std::mutex> lock(gMutex);

	if (!gInitialized) {
		return nullptr;
	}

	const char* firstWord = table->GetFirstWord(context);
	int wordCount = table->GetWordCount(context);
	if (firstWord == nullptr || wordCount <= 1 || std::string(firstWord) != gSettings.triggerWord) {
		return nullptr;
	}

	std::unique_ptr<MatchHandle> result(new MatchHandle());
	for (int i = 0; i < static_cast<int>(gEntries.size()) &&
		static_cast<int>(result->entryIndexes.size()) < gSettings.searchLimit; ++i) {
		const Entry& entry = gEntries[i];
		int level = table->Match(context, entry.searchableText.c_str(), 1);
		if (level == -1) {
			continue;
		}

		// コマンド名が合致しているので、少なくとも前方一致相当にする
		if (level == 3) {
			level = 4;
		}

		result->entryIndexes.push_back(i);
		result->matchLevels.push_back(level);
	}

	if (result->entryIndexes.empty()) {
		// ダミーの検索結果を追加
		result->entryIndexes.push_back(0);
		result->matchLevels.push_back(1);   // 1:HiddenMatch
	}

	return result.release();
}

int CanExecute(LNCRPLUGINMATCHHANDLE handle, int index)
{
	UNREFERENCED_PARAMETER(handle);
	UNREFERENCED_PARAMETER(index);
	return 1;
}

std::wstring BuildCommandLine(const Entry& entry)
{
	std::vector<std::wstring> arguments;
	for (const auto& argument : gSettings.execute) {
		std::string value = Replace(argument, "{lno}", std::to_string(entry.lineNumber));
		value = Replace(value, "{file}", entry.filePath);
		arguments.push_back(Utf8ToWide(value));
	}

	std::wstring commandLine;
	for (size_t i = 0; i < arguments.size(); ++i) {
		if (i != 0) {
			commandLine += L' ';
		}
		commandLine += QuoteCommandArgument(arguments[i]);
	}
	return commandLine;
}

int Execute(LNCRPLUGINMATCHHANDLE handle, int index, int argc, char** argv)
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);
	const Entry* entry = nullptr;
	if (!GetEntry(handle, index, &entry)) {
		return SetError(handle, "Invalid search result.");
	}

	std::wstring commandLine;
	{
		std::lock_guard<std::mutex> lock(gMutex);
		commandLine = BuildCommandLine(*entry);
	}
	std::vector<wchar_t> mutableCommand(commandLine.begin(), commandLine.end());
	mutableCommand.push_back(L'\0');
	STARTUPINFOW startup{};
	startup.cb = sizeof(startup);
	PROCESS_INFORMATION process{};
	if (!CreateProcessW(nullptr, mutableCommand.data(), nullptr, nullptr, FALSE,
		CREATE_UNICODE_ENVIRONMENT, nullptr, nullptr, &startup, &process)) {
		return SetError(handle, "Failed to start the editor.");
	}
	CloseHandle(process.hThread);
	CloseHandle(process.hProcess);
	return 0;
}

int GetErrorStringApi(LNCRPLUGINMATCHHANDLE handle, int index, char* buffer, size_t len)
{
	const Entry* entry = nullptr;
	if (!GetEntry(handle, index, &entry)) {
		return -1;
	}
	auto* match = static_cast<MatchHandle*>(handle);
	std::lock_guard<std::mutex> lock(match->mutex);
	const std::string value = match->error;
	const size_t required = value.size() + 1;
	if (len == 0) {
		return static_cast<int>(required);
	}
	const size_t copied = (std::min)(len, required);
	memcpy(buffer, value.c_str(), copied);
	buffer[copied - 1] = '\0';
	return static_cast<int>(copied);
}


int GetIcon(LNCRPLUGINMATCHHANDLE handle, int index, HICON* icon)
{
	UNREFERENCED_PARAMETER(handle);
	UNREFERENCED_PARAMETER(index);

	int ret = 1;
	if (icon) {
		static HICON hIcon = LoadIcon(g_hModule, MAKEINTRESOURCE(IDI_ICON1));
		*icon = hIcon;
		ret = hIcon ? 0 : 1;
	}
	return ret;
}

} // namespace

namespace memogrep {

bool Initialize()
{
	return LoadIndex();
}

void Finalize()
{
	std::lock_guard<std::mutex> lock(gMutex);
	gEntries.clear();
	gSettings = Settings();
	gLauncherFunctions = LAUNCHER_FUNCTION_TABLE();
	gInitialized = false;
}

} // namespace memogrep

extern "C" int LNCRPLUGIN_API LNCRPLUGIN_Bind(int version, LNCRPLUGIN_EXPORTTABLE* table)
{
	if (version != PLUGINVERSION || table == nullptr) {
		return 1;
	}
	table->Initialize = &Initialize;
	table->Query = &Query;
	table->GetMatchCount = &GetMatchCount;
	table->GetMatchLevel = &GetMatchLevel;
	table->CloseHandle = &CloseMatchHandle;
	table->GetName = [](LNCRPLUGINMATCHHANDLE h, int i, char* b, size_t l) {
		return GetString(h, i, b, l, &GetName);
	};
	table->GetDescription = [](LNCRPLUGINMATCHHANDLE h, int i, char* b, size_t l) {
		return GetString(h, i, b, l, &GetDescription);
	};
	table->GetGuide = [](LNCRPLUGINMATCHHANDLE h, int i, char* b, size_t l) {
		return GetString(h, i, b, l, &GetGuide);
	};
	table->GetTypeDisplayName = [](LNCRPLUGINMATCHHANDLE h, int i, char* b, size_t l) {
		return GetString(h, i, b, l, &GetTypeDisplayName);
	};
	table->CanExecute = &CanExecute;
	table->Execute = &Execute;
	table->GetErrorString = &GetErrorStringApi;
	table->GetIcon = &GetIcon;
	table->Finalize = &memogrep::Finalize;
	return 0;
}
