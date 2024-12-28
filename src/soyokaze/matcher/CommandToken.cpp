#include "pch.h"
#include "CommandToken.h"

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
		auto c = mCommandStr[i];
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

size_t CommandToken::GetCount() const
{
	return mTokenPos.size(); 
}

}
}

