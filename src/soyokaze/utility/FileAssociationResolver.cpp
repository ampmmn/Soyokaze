#include "pch.h"
#include "FileAssociationResolver.h"
#include "utility/CommandLineArgument.h"
#include "utility/Path.h"
#include <shlwapi.h>

#pragma comment(lib, "shlwapi.lib")

namespace launcherapp {
namespace utility {

namespace {

/**
  コマンドインタープリターのパスを取得する
  @param[out] path cmd.exeのパス
  @return パスを取得できた場合はtrue
*/
bool GetCommandInterpreterPath(CString& path)
{
	DWORD requiredLength = GetEnvironmentVariable(_T("ComSpec"), nullptr, 0);
	if (requiredLength > 0) {
		std::vector<TCHAR> buffer(requiredLength);
		DWORD length = GetEnvironmentVariable(_T("ComSpec"), buffer.data(), requiredLength);
		if (length > 0 && length < requiredLength) {
			CString candidate(buffer.data());
			if (Path::FileExists(candidate)) {
				path = candidate;
				return true;
			}
		}
	}

	TCHAR systemDirectory[MAX_PATH_NTFS]{};
	UINT directoryLength = GetSystemDirectory(systemDirectory, _countof(systemDirectory));
	if (directoryLength == 0 || directoryLength >= _countof(systemDirectory)) {
		return false;
	}
	PathAppend(systemDirectory, _T("cmd.exe"));
	path = systemDirectory;
	return Path::FileExists(path) != FALSE;
}

} // 名前空間

bool FileAssociationResolver::Resolve(
	const CString& filePath,
	const CString& arguments,
	CString& executablePath,
	CString& commandLine
) const
{
	if (filePath.IsEmpty() || Path::FileExists(filePath) == FALSE) {
		return false;
	}

	CString extension(PathFindExtension(filePath));
	if (extension.CompareNoCase(_T(".exe")) == 0 || extension.CompareNoCase(_T(".com")) == 0) {
		executablePath = filePath;
		commandLine = arguments;
		return true;
	}
	if (extension.CompareNoCase(_T(".bat")) == 0 || extension.CompareNoCase(_T(".cmd")) == 0) {
		if (GetCommandInterpreterPath(executablePath) == false) {
			return false;
		}

		commandLine = _T("/d /s /c \"") + launcherapp::utility::QuoteArgument(filePath);
		if (arguments.IsEmpty() == FALSE) {
			commandLine += _T(" ");
			commandLine += arguments;
		}
		commandLine += _T("\"");
		return true;
	}

	CString associationCommand;
	if (QueryAssociationCommand(extension, associationCommand) == false ||
		ParseAssociationCommand(associationCommand, executablePath, commandLine) == false) {
		return false;
	}

	// 関連付けコマンドに含まれる環境変数を展開する
	std::vector<TCHAR> expandedPath(32768);
	DWORD expandedLength = ExpandEnvironmentStrings(executablePath, expandedPath.data(), (DWORD)expandedPath.size());
	if (expandedLength == 0 || expandedLength > expandedPath.size()) {
		return false;
	}
	executablePath = expandedPath.data();

	bool hasArgumentPlaceholder = commandLine.Find(_T("%*")) >= 0;
	bool hasFilePlaceholder = commandLine.Find(_T("%1")) >= 0 || commandLine.Find(_T("%L")) >= 0;
	ReplacePlaceholder(commandLine, _T("%1"), filePath, true);
	ReplacePlaceholder(commandLine, _T("%L"), filePath, true);
	ReplacePlaceholder(commandLine, _T("%*"), arguments, false);
	if (hasFilePlaceholder == false) {
		if (commandLine.IsEmpty() == FALSE) {
			commandLine += _T(" ");
		}
		commandLine += launcherapp::utility::QuoteArgument(filePath);
	}
	if (hasArgumentPlaceholder == false && arguments.IsEmpty() == FALSE) {
		if (commandLine.IsEmpty() == FALSE) {
			commandLine += _T(" ");
		}
		commandLine += arguments;
	}

	return executablePath.IsEmpty() == FALSE;
}

bool FileAssociationResolver::ParseAssociationCommand(
	const CString& command,
	CString& executablePath,
	CString& arguments
)
{
	CString trimmed(command);
	trimmed.Trim();
	if (trimmed.IsEmpty()) {
		return false;
	}

	if (trimmed[0] == _T('"')) {
		int endQuote = trimmed.Find(_T('"'), 1);
		if (endQuote < 0) {
			return false;
		}
		executablePath = trimmed.Mid(1, endQuote - 1);
		arguments = trimmed.Mid(endQuote + 1);
	}
	else {
		int separator = trimmed.FindOneOf(_T(" \t"));
		if (separator < 0) {
			executablePath = trimmed;
			arguments.Empty();
		}
		else {
			executablePath = trimmed.Left(separator);
			arguments = trimmed.Mid(separator);
		}
	}
	arguments.TrimLeft();
	return executablePath.IsEmpty() == FALSE;
}

bool FileAssociationResolver::QueryAssociationCommand(const CString& extension, CString& command)
{
	if (extension.IsEmpty()) {
		return false;
	}

	DWORD bufferSize = 256;
	std::vector<TCHAR> buffer(bufferSize);
	for (;;) {
		DWORD actualSize = bufferSize;
		HRESULT result = AssocQueryString(
			ASSOCF_NONE,
			ASSOCSTR_COMMAND,
			extension,
			_T("open"),
			buffer.data(),
			&actualSize
		);
		if (result == S_FALSE && actualSize > bufferSize) {
			bufferSize = actualSize;
			buffer.resize(bufferSize);
			continue;
		}
		if (FAILED(result)) {
			return false;
		}
		command = buffer.data();
		return command.IsEmpty() == FALSE;
	}
}

void FileAssociationResolver::ReplacePlaceholder(
	CString& text,
	LPCTSTR placeholder,
	const CString& value,
	bool quoteValue
)
{
	int position = 0;
	const int placeholderLength = (int)_tcslen(placeholder);
	while ((position = text.Find(placeholder, position)) >= 0) {
		bool alreadyQuoted = position > 0 && position + placeholderLength < text.GetLength() &&
			text[position - 1] == _T('"') && text[position + placeholderLength] == _T('"');
		CString replacement(value);
		if (quoteValue && alreadyQuoted == false && replacement.IsEmpty() == FALSE) {
			replacement = _T("\"") + replacement + _T("\"");
		}
		text.Delete(position, placeholderLength);
		text.Insert(position, replacement);
		position += replacement.GetLength();
	}
}

}
}
