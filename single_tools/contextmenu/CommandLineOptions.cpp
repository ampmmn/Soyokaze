#include "framework.h"
#include "CommandLineOptions.h"

#include <string>

namespace {

const wchar_t POSITION_OPTION_PREFIX[] = L"--position=";

/**
  文字列を10進整数として解析する
  @param[in]  text  解析対象の文字列
  @param[out] value 解析結果
  @return true:成功 false:不正な文字列
*/
bool ParseInteger(const std::wstring& text, long& value)
{
	if (text.empty()) {
		return false;
	}

	size_t index = 0;
	bool negative = false;
	if (text[0] == L'-' || text[0] == L'+') {
		negative = text[0] == L'-';
		index = 1;
	}
	if (index >= text.size()) {
		return false;
	}

	long result = 0;
	for (; index < text.size(); ++index) {
		const wchar_t character = text[index];
		if (character < L'0' || character > L'9') {
			return false;
		}
		result = result * 10 + (character - L'0');
		if (result > 1000000) {
			// 座標として意味のない値は受け付けない
			return false;
		}
	}

	value = negative ? -result : result;
	return true;
}

/**
  --position=<x>,<y> の指定内容を解析する
  @param[in]  text     解析対象の文字列
  @param[out] position 解析結果の座標
  @return true:成功 false:不正な文字列
*/
bool ParsePosition(const std::wstring& text, POINT& position)
{
	const size_t separator = text.find(L',');
	if (separator == std::wstring::npos) {
		return false;
	}

	long x = 0;
	long y = 0;
	if (!ParseInteger(text.substr(0, separator), x)) {
		return false;
	}
	if (!ParseInteger(text.substr(separator + 1), y)) {
		return false;
	}

	position.x = static_cast<LONG>(x);
	position.y = static_cast<LONG>(y);
	return true;
}

} // 名前空間

struct CommandLineOptions::PImpl
{
	std::vector<std::wstring> paths;
	bool hasPosition = false;
	POINT position{ 0, 0 };
};

CommandLineOptions::CommandLineOptions()
	: in(std::make_unique<PImpl>())
{
}

CommandLineOptions::~CommandLineOptions() = default;

void CommandLineOptions::Parse(int argc, LPWSTR* argv)
{
	in->paths.clear();
	in->hasPosition = false;
	in->position = POINT{ 0, 0 };

	const std::wstring prefix(POSITION_OPTION_PREFIX);
	for (int i = 0; i < argc; ++i) {
		if (argv[i] == nullptr) {
			continue;
		}
		const std::wstring argument(argv[i]);
		if (argument.empty()) {
			continue;
		}

		// --position=<x>,<y> は表示位置の指定として扱う
		if (argument.compare(0, prefix.size(), prefix) == 0) {
			POINT position{ 0, 0 };
			if (ParsePosition(argument.substr(prefix.size()), position)) {
				in->hasPosition = true;
				in->position = position;
			}
			continue;
		}

		// --position 以外の引数は対象パスとして扱う
		in->paths.push_back(argument);
	}
}

const std::vector<std::wstring>& CommandLineOptions::GetPaths() const
{
	return in->paths;
}

bool CommandLineOptions::HasPosition() const
{
	return in->hasPosition;
}

POINT CommandLineOptions::GetPosition() const
{
	return in->position;
}
