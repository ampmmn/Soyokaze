#include "pch.h"
#include "CommandToken.h"
#include "utility/Path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace matcher {


CommandToken::CommandToken(const CString& commandStr) : mCommandStr(commandStr)
{
	std::vector<int> tmpPos;

	bool inToken = false;
	bool inQuate = false;
	int len = mCommandStr.GetLength();
	for (int i = 0; i < len; ++i) {
		auto& c = mCommandStr[i];
		if (inQuate == false && c == _T('"')) {
		 	inQuate = true;
			inToken = true;
			tmpPos.push_back(i);
			continue;
	 	}
		if (inQuate && c == _T('"')) {
		 	inQuate = false;
			continue;
	 	}
		if (inQuate == false && c == _T(' ')) {
			inToken = false;
			continue;
		}
		else {
			if (inToken) {
				continue;
			}
			tmpPos.push_back(i);
			inToken = true;
		}
	}

	mTokenPos.swap(tmpPos);
}

bool CommandToken::GetTrailingString(int endPos, CString& trailingText)
{
	// 文字列の区切り位置と基準となる位置(endPos)を比較して、
	// 区切り位置がendPosより後ろにある場合はそれ以降の部分文字列を返す
	auto it = mTokenPos.begin();
	for (; it != mTokenPos.end(); ++it) {
		auto pos = *it;
		if (endPos <= pos) {
			// 区切り位置がendPosより後ろにあったので探索終了 
			break;
		}
	}

	// 末尾に達した場合
	if (it == mTokenPos.end()) {
		return false;
	}

	// 部分文字列の取り出し
	trailingText = mCommandStr.Mid(*it);

	return true;
}

bool CommandToken::GetTokenRange(int position, int& startPos, int& endPos) const
{
	if (mTokenPos.empty()) {
		return false;
	}

	for (size_t i = 0; i < mTokenPos.size(); ++i) {
		int start = mTokenPos[i];
		int end = i + 1 < mTokenPos.size() ? mTokenPos[i + 1] : mCommandStr.GetLength();
		while (end > start && mCommandStr[end - 1] == _T(' ')) {
			--end;
		}
		if (position >= start && position <= end) {
			startPos = start;
			endPos = end;
			return true;
		}
	}
	return false;
}

static bool IsAbsolutePathStart(const CString& text, int startPos)
{
	int length = text.GetLength();
	if (startPos < 0 || length <= startPos) {
		return false;
	}

	if (startPos + 1 < length && text[startPos] == _T('\\') && text[startPos + 1] == _T('\\')) {
		return true;
	}
	if (startPos + 2 < length &&
		((text[startPos] >= _T('A') && text[startPos] <= _T('Z')) ||
		 (text[startPos] >= _T('a') && text[startPos] <= _T('z'))) &&
		text[startPos + 1] == _T(':') &&
		(text[startPos + 2] == _T('\\') || text[startPos + 2] == _T('/'))) {
		return true;
	}
	return false;
}

bool CommandToken::GetPathParameterRange(int& startPos, int& endPos) const
{
	if (mCommandStr.IsEmpty() || mCommandStr[0] == _T('"')) {
		return false;
	}

	int commandEnd = -1;
	if (IsAbsolutePathStart(mCommandStr, 0)) {
		// 絶対パスで始まる入力では、実在するファイルの後ろに引数があるか確認する
		CString path;
		int separatorPos = 0;
		while ((separatorPos = mCommandStr.Find(_T(' '), separatorPos)) != -1) {
			path = mCommandStr.Left(separatorPos);
			path.TrimRight();
			if (Path::FileExists(path) && Path::IsDirectory(path) == false) {
				commandEnd = separatorPos + 1;
				while (commandEnd < mCommandStr.GetLength() && mCommandStr[commandEnd] == _T(' ')) {
					++commandEnd;
				}
				break;
			}
			++separatorPos;
		}
	}

	startPos = -1;
	if (commandEnd != -1) {
		if (IsAbsolutePathStart(mCommandStr, commandEnd)) {
			startPos = commandEnd;
		}
		else {
			int separatorPos = commandEnd;
			while ((separatorPos = mCommandStr.Find(_T(' '), separatorPos)) != -1) {
				++separatorPos;
				while (separatorPos < mCommandStr.GetLength() && mCommandStr[separatorPos] == _T(' ')) {
					++separatorPos;
				}
				if (IsAbsolutePathStart(mCommandStr, separatorPos)) {
					startPos = separatorPos;
					break;
				}
			}
		}
	}
	else if (IsAbsolutePathStart(mCommandStr, 0)) {
		startPos = 0;
	}
	else {
		int separatorPos = 0;
		while ((separatorPos = mCommandStr.Find(_T(' '), separatorPos)) != -1) {
			++separatorPos;
			while (separatorPos < mCommandStr.GetLength() && mCommandStr[separatorPos] == _T(' ')) {
				++separatorPos;
			}
			if (IsAbsolutePathStart(mCommandStr, separatorPos)) {
				startPos = separatorPos;
				break;
			}
		}
	}

	if (startPos == -1 || mCommandStr.Find(_T(' '), startPos) == -1) {
		return false;
	}

	endPos = mCommandStr.GetLength();
	while (endPos > startPos && mCommandStr[endPos - 1] == _T(' ')) {
		--endPos;
	}
	return endPos > startPos;
}

bool CommandToken::GetToken(int index, CString& token) const
{
	if (index < 0 || index >= static_cast<int>(mTokenPos.size())) {
		return false;
	}
	int start = mTokenPos[index];
	int end = index + 1 < static_cast<int>(mTokenPos.size()) ? mTokenPos[index + 1] : mCommandStr.GetLength();
	while (end > start && mCommandStr[end - 1] == _T(' ')) {
		--end;
	}
	token = mCommandStr.Mid(start, end - start);
	return true;
}

size_t CommandToken::GetCount() const
{
	return mTokenPos.size(); 
}

}
}

