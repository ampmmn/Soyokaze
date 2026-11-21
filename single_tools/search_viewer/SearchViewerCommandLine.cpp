#include "SearchViewerCommandLine.h"

#include "SearchViewerConfig.h"

#include <algorithm>
#include <utility>

bool SearchViewerCommandLine::Parse(const std::vector<std::wstring>& arguments, Options& options, std::wstring& error)
{
	Options parsed;
	bool hasConfig = false;
	std::vector<std::wstring> queryParts;
	for (size_t i = 0; i < arguments.size(); ++i) {
		const std::wstring& argument = arguments[i];
		if (argument.compare(0, 10, L"--profile=") == 0) {
			parsed.profiles.push_back(argument.substr(10));
		}
		else if (argument == L"--profile") {
			if (i + 1 >= arguments.size()) {
				error = L"--profile の指定が不正です。";
				return false;
			}
			parsed.profiles.push_back(arguments[++i]);
		}
		else if (argument.compare(0, 9, L"--config=") == 0) {
			if (hasConfig) {
				error = L"--config は1回だけ指定できます。";
				return false;
			}
			parsed.configPath = argument.substr(9);
			hasConfig = true;
		}
		else if (argument == L"--config") {
			if (hasConfig || i + 1 >= arguments.size()) {
				error = L"--config の指定が不正です。";
				return false;
			}
			parsed.configPath = arguments[++i];
			hasConfig = true;
		}
		else if (argument.empty() == false && argument[0] == L'-') {
			error = L"未対応のオプションです: " + argument;
			return false;
		}
		else {
			queryParts.push_back(argument);
		}
	}

	if (parsed.profiles.empty()) {
		error = L"--profile に有効なプロファイル名を1つ以上指定してください。";
		return false;
	}
	for (const auto& profile : parsed.profiles) {
		if (SearchViewerConfig::IsValidProfileName(profile) == false) {
			error = L"--profile に有効なプロファイル名を指定してください。";
			return false;
		}
		if (std::count(parsed.profiles.begin(), parsed.profiles.end(), profile) > 1) {
			error = L"同じプロファイルを複数回指定できません: " + profile;
			return false;
		}
	}
	if (queryParts.size() != 1) {
		error = L"検索語は1つの引数として指定してください。";
		return false;
	}

	parsed.query = queryParts.front();
	options = std::move(parsed);
	error.clear();
	return true;
}

std::wstring SearchViewerCommandLine::QuoteArgument(const std::wstring& argument)
{
	std::wstring quoted(1, L'"');
	size_t backslashCount = 0;
	for (const wchar_t character : argument) {
		if (character == L'\\') {
			++backslashCount;
		}
		else if (character == L'"') {
			quoted.append(backslashCount * 2 + 1, L'\\');
			quoted.push_back(L'"');
			backslashCount = 0;
		}
		else {
			quoted.append(backslashCount, L'\\');
			quoted.push_back(character);
			backslashCount = 0;
		}
	}
	quoted.append(backslashCount * 2, L'\\');
	quoted.push_back(L'"');
	return quoted;
}
